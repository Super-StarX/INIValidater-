﻿#include "Helper.h"
#include "ProgressBar.h"
#include <iostream>

ProgressBar ProgressBar::INIFileProgress;
ProgressBar ProgressBar::CheckerProgress;
int ProgressBar::line = 2;

double ProgressData::getPercent() const {
	return total > 0 ? (double)processed / total * 100 : 0.0;
}

auto ProgressData::getElapsed() const {
	auto time = finished ? endTime : std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(time - startTime).count();
}

ProgressBar::~ProgressBar() {
	stop(); // 确保析构时停止线程
}

void ProgressBar::addProgressBar(size_t id, const std::string& name, size_t total) {
	constexpr size_t fileNameWidth = 25;

	std::lock_guard<std::mutex> lock(mutex);
	progressBars[id].total = total;
	progressBars[id].line = line++;
	progressBars[id].name = string::clamp(name, 25);
	progressBars[id].startTime = std::chrono::steady_clock::now();
	start(); // 启动显示线程
}

void ProgressBar::updateProgress(size_t id, size_t processed) {
	std::lock_guard<std::mutex> lock(mutex);
	if (progressBars.contains(id))
		progressBars[id].processed = processed;
}

double ProgressBar::getPercent(size_t id) {
	return progressBars.contains(id) ? progressBars[id].getPercent() : 100.0;
}

void ProgressBar::markFinished(size_t id) {
	std::lock_guard<std::mutex> lock(mutex);
	if (progressBars.contains(id)) {
		progressBars[id].endTime = std::chrono::steady_clock::now();
		progressBars[id].finished = true;
		progressBars[id].processed = progressBars[id].total;
	}
}

void ProgressBar::stop() {
	if (threadStarted) {
		run();
		std::cout << "\033[?25h";
		stopFlag = true;
		if (displayThread.joinable())
			displayThread.join();
	}
}

void ProgressBar::start() {
	if (!threadStarted) {
		std::cout << "\033[?25l";
		threadStarted = true;
		displayThread = std::thread(&ProgressBar::loop, this);
	}
}

void ProgressBar::run() {
	try {
		std::lock_guard<std::mutex> lock(mutex);
		for (const auto& [id, progress] : progressBars) {
			double percent = progress.getPercent();
			auto elapsed = progress.getElapsed();

			// 固定文件名宽度
			constexpr size_t total = 50;

			std::cerr << "\033[" << progress.line << ";0H"; // 定位光标到行首

			// 渲染进度条
			size_t completed = (size_t)(percent / 2);
			size_t remain = total - completed;
			if (progress.finished)
				std::cerr << progress.name << std::format("[\033[32m{0:━<{1}}>\033[91m{2:┈<{3}}\033[0m]", "", completed, "", remain);
			else
				std::cerr << progress.name << std::format("[\033[32m{0:━<{1}}>\033[90m{2:┈<{3}}\033[0m]", "", completed, "", remain);

			// 显示百分比和时间
			std::cerr << std::fixed << std::setprecision(2) << percent << "% (" << elapsed << "ms)\n";
		}
	}
	catch (const std::exception&) {
	}
}

void ProgressBar::loop() {
	while (!stopFlag) {
		run();
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
}

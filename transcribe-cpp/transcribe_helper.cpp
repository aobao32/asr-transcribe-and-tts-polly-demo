#include "transcribe_helper.h"
#include <mutex>

std::string TranscribeHelper::currentResult = "";
bool TranscribeHelper::newResultAvailable = false;
static std::mutex resultMutex;

std::string TranscribeHelper::getTranscriptionResult() {
    std::lock_guard<std::mutex> lock(resultMutex);
    newResultAvailable = false;
    return currentResult;
}

void TranscribeHelper::setTranscriptionResult(const std::string& result) {
    std::lock_guard<std::mutex> lock(resultMutex);
    currentResult = result;
    newResultAvailable = true;
}

bool TranscribeHelper::hasNewResult() {
    std::lock_guard<std::mutex> lock(resultMutex);
    return newResultAvailable;
}

void TranscribeHelper::clearResult() {
    std::lock_guard<std::mutex> lock(resultMutex);
    currentResult.clear();
    newResultAvailable = false;
}
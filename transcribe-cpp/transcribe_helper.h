#pragma once
#include <string>
#include <vector>

class TranscribeHelper {
public:
    static std::string getTranscriptionResult();
    static void setTranscriptionResult(const std::string& result);
    static bool hasNewResult();
    static void clearResult();
    
private:
    static std::string currentResult;
    static bool newResultAvailable;
};
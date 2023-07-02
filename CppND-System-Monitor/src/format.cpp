#include <string>

#include "format.h"

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) { 
    string time_h_m_s;
    int hh, mm, ss;
    mm = seconds/60;
    hh = mm/60;
    time_h_m_s = std::to_string(int(hh)) + ":" + std::to_string(int(mm%60)) + ":" + std::to_string(int(seconds%60));
    return time_h_m_s;
}

#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line, key;
  int value;
  
  float MemTotal ,MemFree, Buffers;
    
  std::ifstream filestream (kProcDirectory+kMeminfoFilename);
    if (filestream.is_open())
    {
      while (std::getline(filestream,line)){
      std::replace(line.begin(),line.end(),':',' ');
      std::istringstream linestream (line);
      while (linestream >> key>>value){
        if (key == "MemTotal"){MemTotal = value;}
        else if (key == "MemFree"){MemFree = value;}
        else if (key =="Buffers"){Buffers = value;}
      }
    }
  } 
  return (1.0 - (MemFree/(MemTotal-Buffers)));
 }

// TODO: Read and return the system uptime
long LinuxParser::UpTime() { 

  long SystemUpTime, Time_Idle;
  std::ifstream myfile (kProcDirectory+kUptimeFilename);
  myfile >> SystemUpTime >> Time_Idle;
  return SystemUpTime;

}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 
  return LinuxParser::ActiveJiffies()+LinuxParser::IdleJiffies();
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) { 
//Refer to this link: https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599

//long systime = UpTime();
string value;
string line;
vector<string> valueGroup;
//long seconds;
long numerator;
float utime,stime ,cutime ,cstime;
std::ifstream filestream (kProcDirectory+to_string(pid)+kStatFilename);

if (filestream.is_open())
{
  std::getline(filestream,line);
  std::istringstream linestream (line);
  for (int j = 0; j<22; j++){
    linestream>>value;
    if (j == 13 || j ==14|| j==15 || j ==16 || j == 21)
    {
      valueGroup.push_back(value);
    }
  }
}
  utime = stol(valueGroup[0]);
  stime = stol(valueGroup[1]);
  cutime = stol(valueGroup[2]);
  cstime = stol(valueGroup[3]);
  //starttime = stol(valueGroup[4]);
    
  // seconds = systime - (starttime/HZ);
  numerator = (utime+stime+cutime+cstime);
  return numerator/sysconf(_SC_CLK_TCK);
}
// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
// unsigned long long int idlealltime = idletime + ioWait;  # ioWait is added in the idleTime
// unsigned long long int systemalltime = systemtime + irq + softIrq;
// unsigned long long int virtalltime = guest + guestnice;
// unsigned long long int totaltime = usertime + nicetime + systemalltime + idlealltime + steal + virtalltime;
//
//   user    nice   system  idle      iowait irq   softirq  steal  guest  guest_nice
// cpu  74608   2520   24433   1117073   6176   4054  0        0      0      0

auto jiffies = CpuUtilization();
return stol(jiffies[CPUStates::kUser_])+stol(jiffies[CPUStates::kNice_])+stol(jiffies[CPUStates::kSystem_])+
        stol(jiffies[CPUStates::kIRQ_])+stol(jiffies[CPUStates::kSoftIRQ_])+
        stol(jiffies[CPUStates::kSteal_])+
        stol(jiffies[CPUStates::kGuest_])+
        stol (jiffies[CPUStates::kGuestNice_]);
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {   
  auto jiffies = CpuUtilization();
  return (stol(jiffies[CPUStates::kIdle_])+stol(jiffies[CPUStates::kIOwait_]));
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
  string line, cpu, value, key;
  vector<string> jiffies;
  std::ifstream filestream (kProcDirectory+kStatFilename);
    if (filestream.is_open())
    {
      while (std::getline(filestream,line)){
        std::istringstream linestream (line);
        linestream >> cpu;
        while(linestream >> key>> value){
          jiffies.push_back(value);
        }
        }
    }
  return jiffies;
}


// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  string key,value, line;
  std::ifstream filestream (kProcDirectory+kStatFilename);
  if (filestream.is_open())
  {
      while (std::getline(filestream,line)){
        std::istringstream linestream (line);
        linestream >> key>> value;
        if (key == "processes"){
          return stoi(value);
          //break;
        }
      }
  }
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string key,value, line;
  std::ifstream filestream (kProcDirectory+kStatFilename);
  if (filestream.is_open())
  {
      while (std::getline(filestream,line)){
        std::istringstream linestream (line);
        linestream >> key>> value;
        if (key == "procs_running"){
          return stoi(value);
          //break;
        }
      }
  }
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) { 
  string value, line, key;
  string command = "";
  std::ifstream filestream (kProcDirectory+to_string(pid)+kCmdlineFilename);
  if (filestream.is_open())
  {
    while (std::getline(filestream,line)){ 
    std::istringstream linestream (line);
    linestream >> key;
    command +=key;
 }
}
return command;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) { 

  string key, value, unit, line;
  std::ifstream filestream (kProcDirectory+to_string(pid)+kStatusFilename);
  if (filestream.is_open())
  {
      while (std::getline(filestream,line)){
        std::replace(line.begin(),line.end(),':',' ');
        std::istringstream linestream (line);
        linestream >> key >> value >> unit;
        if (key == "VmSize"){
          return value;
        }
      }
  }
}  

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) { 
  string key, value;
  string line;
  std::ifstream filestream (kProcDirectory+to_string(pid)+kStatFilename);
  std::ifstream filestream2 (kPasswordPath);
  if (filestream.is_open())
  {
      while (std::getline(filestream,line)){
        std::replace(line.begin(),line.end(),':',' ');
        std::istringstream linestream (line);
        linestream >> key >> value;
        if (key == "UiD"){
          return value;
        }
      }
  } 
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {  
  string key, value,value1, line;
  std::ifstream filestream (kPasswordPath);
  string uid = Uid(pid);
  if(filestream.is_open()){
    while (std::getline(filestream,line)){
        std::replace(line.begin(),line.end(),':',' ');
        std::istringstream linestream (line);
        linestream >>key>>value>>value1;
        if (uid == value1){
          return key;
        }        
      }
  }
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) { 

std::ifstream filestream (kProcDirectory+to_string(pid)+kStatFilename);
string line, value;
vector<string> linestring;
long time;
if (filestream.is_open())
{
  while (std::getline(filestream,line)){
    std::istringstream linestream (line);
     while (linestream >> value) {
        linestring.push_back(value);
      }
  }
} 
time = std::stol(linestring[21])/sysconf(_SC_CLK_TCK);
return time;
}

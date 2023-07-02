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
  
  int MemTotal ,MemFree, Buffers, Cached, SReclaimable ,Shmem,SwapTotal, SwapFree;
  float MemGreen, MemBlue, MemYellow, Swap;
  
  std::ifstream filestream (kProcDirectory+kMeminfoFilename);
    if (filestream.is_open())
    {
      while (std::getline(filestream,line)){
      std::replace(line.begin(),line.end(),':',' ');
      std::istringstream linestream (line);
      while (linestream >> key>>value){
        if (key == " MemTotal"){MemTotal = value;}
        else if (key == "MemFree"){MemFree = value;}
        else if (key =="Buffers"){Buffers = value;}
        else if (key == "Cached"){Cached = value;}
        else if (key == "SReclaimable"){SReclaimable = value;}
        else if (key =="Shmem"){Shmem = value;}
        else if (key == "SwapTotal"){SwapTotal = value;}
        else if (key == "SwapFree"){SwapFree = value;}
      }
    }
    MemGreen = (MemTotal-MemFree) - (Buffers+Cached);
    MemBlue = Buffers;
    MemYellow = Cached + SReclaimable - Shmem;
    Swap = SwapTotal - SwapFree;
  } 
  return MemGreen;
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
  return (ActiveJiffies()+IdleJiffies());
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) { 
//Refer to this link: https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599

long systime = UpTime();
long cpu_usage;
string value;
string line;
vector<string> valueGroup;
long seconds;
long HZ;
float utime,stime ,cutime ,cstime ,starttime;
std::ifstream filestream (kProcDirectory+to_string(pid)+kStatFilename);

if (filestream.is_open())
{
  std::getline(filestream,line);
  std::istringstream linestream (line);
  for (int j = 0; j<23; j++){
    linestream>>value;
    if (j == 14 || j ==15|| j==16 || j ==17 || j == 22)
    {
      valueGroup.push_back(value);
    }
  }
}
  utime = stof(valueGroup[0]);
  stime = stof(valueGroup[1]);
  cutime = stof(valueGroup[2]);
  cstime = stof(valueGroup[3]);
  starttime = stof(valueGroup[4]);
    
  HZ = sysconf(_SC_CLK_TCK);
  seconds = systime - (starttime/HZ);
  cpu_usage = 100*(((utime+stime+cutime+cstime)/HZ)/seconds);
  return cpu_usage;
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
        stol(jiffies[CPUStates::kSteal_])+stol(jiffies[CPUStates::kGuest_])+
        stol (jiffies[CPUStates::kGuestNice_]);
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {   
  auto jiffies = CpuUtilization();
  stol(jiffies[CPUStates::kIdle_])+stol(jiffies[CPUStates::kIOwait_]);
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
          break;
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
          break;
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
  std::ifstream filestream (kProcDirectory+to_string(pid)+kStatFilename);
  if (filestream.is_open())
  {
      while (std::getline(filestream,line)){
        std::replace(line.begin(),line.end(),':',' ');
        std::istringstream linestream (line);
        linestream >> key >> value >> unit;
        if (key == "VmSize"){
          return (value+" "+unit);
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
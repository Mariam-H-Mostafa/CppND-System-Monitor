#include "processor.h"
#include "linux_parser.h"

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() { 
  long Total = LinuxParser::ActiveJiffies();
  long Idle = LinuxParser::IdleJiffies();
  return ((Total-Idle)/Total); 
 }
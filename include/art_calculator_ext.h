#ifndef __ART_CALCULATOR_EXT_H__
#define __ART_CALCULATOR_EXT_H__
#include <string>
namespace PLATFORM {
	int art_calculator_run(std::string s, double& ret, std::string* err = NULL);
	int art_calculator_addfunction(std::string name, double (*func)(double));
}
#endif
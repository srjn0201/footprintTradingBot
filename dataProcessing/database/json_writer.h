#ifndef JSON_WRITER_H
#define JSON_WRITER_H

#include "../dataStructure.h"
#include <string>

void writeContractToJson(const Contract& contract, const std::string& output_dir);

#endif // JSON_WRITER_H

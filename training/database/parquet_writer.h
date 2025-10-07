#ifndef PARQUET_WRITER_H
#define PARQUET_WRITER_H

#include "../dataStructure.h"
#include <string>

void writeContractToParquet(const Contract& contract, const std::string& output_dir);

#endif // PARQUET_WRITER_H

#include "parquet_writer.h"
#include <iostream>
#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/writer.h>

void writeContractToParquet(const Contract& contract, const std::string& output_dir) {
    std::cout << "Writing contract data to Parquet file in " << output_dir << std::endl;

    auto schema = arrow::schema({arrow::field("contractName", arrow::utf8())});

    arrow::StringBuilder contractName_builder;
    PARQUET_THROW_NOT_OK(contractName_builder.Append(contract.contractName));

    std::shared_ptr<arrow::Array> contractName_array;
    PARQUET_THROW_NOT_OK(contractName_builder.Finish(&contractName_array));

    auto table = arrow::Table::Make(schema, {contractName_array});

    std::shared_ptr<arrow::io::FileOutputStream> outfile;
    PARQUET_ASSIGN_OR_THROW(outfile, arrow::io::FileOutputStream::Open(output_dir + "/contract.parquet"));

    PARQUET_THROW_NOT_OK(parquet::arrow::WriteTable(*table, arrow::default_memory_pool(), outfile, 1));
}
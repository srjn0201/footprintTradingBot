#include "parquet_writer.h"
#include <iostream>
#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/writer.h>
#include <arrow/builder.h>
#include <arrow/array.h>
#include <arrow/array/builder_nested.h>
#include <arrow/type.h>

// Simple schema with just essential fields
std::shared_ptr<arrow::Schema> getContractSchema() {
    auto barSchema = arrow::struct_({
        arrow::field("startTime", arrow::int64()),
        arrow::field("endTime", arrow::int64()),
        arrow::field("open", arrow::float64()),
        arrow::field("high", arrow::float64()),
        arrow::field("low", arrow::float64()),
        arrow::field("close", arrow::float64()),
        arrow::field("barTotalVolume", arrow::float64())
    });

    auto daySchema = arrow::struct_({
        arrow::field("dayOfTheWeek", arrow::int64()),
        arrow::field("bars", arrow::list(barSchema))
    });

    auto weekSchema = arrow::struct_({
        arrow::field("weekOfTheContract", arrow::int64()),
        arrow::field("days", arrow::list(daySchema))
    });

    return arrow::schema({
        arrow::field("contractName", arrow::utf8()),
        arrow::field("weeks", arrow::list(weekSchema))
    });
}

void writeContractToParquet(const Contract& contract, const std::string& output_dir) {
    std::cout << "Writing contract data to Parquet file..." << std::endl;
    
    try {
        // Create schema
        auto schema = getContractSchema();
        
        // Create builders
        arrow::MemoryPool* pool = arrow::default_memory_pool();
        arrow::StringBuilder contractNameBuilder(pool);
        
        // Create nested list builder for weeks
        auto weekStructType = std::static_pointer_cast<arrow::ListType>(schema->field(1)->type());
        std::unique_ptr<arrow::ListBuilder> weeksBuilder(
            new arrow::ListBuilder(pool, 
                std::make_shared<arrow::StructBuilder>(
                    weekStructType->value_type()->fields(),
                    pool
                )
            )
        );
        
        // Build contract name
        PARQUET_THROW_NOT_OK(contractNameBuilder.Append(contract.contractName));
        
        // Build weeks
        for (const auto& week : contract.weeks) {
            PARQUET_THROW_NOT_OK(weeksBuilder->Append());
            auto weekStructBuilder = static_cast<arrow::StructBuilder*>(weeksBuilder->value_builder());
            
            // Week of contract
            auto weekTimeBuilder = static_cast<arrow::Int64Builder*>(weekStructBuilder->field_builder(0));
            PARQUET_THROW_NOT_OK(weekTimeBuilder->Append(week.weekOfTheContract));
            
            // Days list
            auto daysBuilder = static_cast<arrow::ListBuilder*>(weekStructBuilder->field_builder(1));
            
            // Build days
            for (const auto& day : week.days) {
                PARQUET_THROW_NOT_OK(daysBuilder->Append());
                auto dayStructBuilder = static_cast<arrow::StructBuilder*>(daysBuilder->value_builder());
                
                // Day of week
                auto dayTimeBuilder = static_cast<arrow::Int64Builder*>(dayStructBuilder->field_builder(0));
                PARQUET_THROW_NOT_OK(dayTimeBuilder->Append(day.dayOfTheWeek));
                
                // Bars list
                auto barsBuilder = static_cast<arrow::ListBuilder*>(dayStructBuilder->field_builder(1));
                
                // Build bars
                for (const auto& bar : day.bars) {
                    PARQUET_THROW_NOT_OK(barsBuilder->Append());
                    auto barStructBuilder = static_cast<arrow::StructBuilder*>(barsBuilder->value_builder());
                    
                    // Bar fields
                    auto startTimeBuilder = static_cast<arrow::Int64Builder*>(barStructBuilder->field_builder(0));
                    auto endTimeBuilder = static_cast<arrow::Int64Builder*>(barStructBuilder->field_builder(1));
                    auto openBuilder = static_cast<arrow::DoubleBuilder*>(barStructBuilder->field_builder(2));
                    auto highBuilder = static_cast<arrow::DoubleBuilder*>(barStructBuilder->field_builder(3));
                    auto lowBuilder = static_cast<arrow::DoubleBuilder*>(barStructBuilder->field_builder(4));
                    auto closeBuilder = static_cast<arrow::DoubleBuilder*>(barStructBuilder->field_builder(5));
                    auto volumeBuilder = static_cast<arrow::DoubleBuilder*>(barStructBuilder->field_builder(6));
                    
                    PARQUET_THROW_NOT_OK(startTimeBuilder->Append(bar.startTime));
                    PARQUET_THROW_NOT_OK(endTimeBuilder->Append(bar.endTime));
                    PARQUET_THROW_NOT_OK(openBuilder->Append(bar.open));
                    PARQUET_THROW_NOT_OK(highBuilder->Append(bar.high));
                    PARQUET_THROW_NOT_OK(lowBuilder->Append(bar.low));
                    PARQUET_THROW_NOT_OK(closeBuilder->Append(bar.close));
                    PARQUET_THROW_NOT_OK(volumeBuilder->Append(bar.barTotalVolume));
                }
            }
        }
        
        // Finish arrays
        std::shared_ptr<arrow::Array> contractNameArray;
        std::shared_ptr<arrow::Array> weeksArray;
        
        PARQUET_THROW_NOT_OK(contractNameBuilder.Finish(&contractNameArray));
        PARQUET_THROW_NOT_OK(weeksBuilder->Finish(&weeksArray));
        
        // Create table
        auto table = arrow::Table::Make(schema, {contractNameArray, weeksArray});
        
        // Write to file
        std::string filename = output_dir + "/contract.parquet";
        std::cout << "Writing to file: " << filename << std::endl;
        
        std::shared_ptr<arrow::io::FileOutputStream> outfile;
        PARQUET_ASSIGN_OR_THROW(outfile, arrow::io::FileOutputStream::Open(filename));
        
        // Write with compression
        parquet::WriterProperties::Builder builder;
        builder.compression(parquet::Compression::SNAPPY);
        
        PARQUET_THROW_NOT_OK(
            parquet::arrow::WriteTable(
                *table,
                arrow::default_memory_pool(),
                outfile,
                1024 * 1024,  // chunk size
                builder.build()
            )
        );
        
        std::cout << "Successfully wrote contract data to parquet file" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error writing parquet file: " << e.what() << std::endl;
        throw;
    }
}
#pragma once
#include <list>
#include "DependencyConsumer.h"
#include "FDAlgorithm.h"
#include "SearchSpace.h"
#include "CSVParser.h"

class Pyro : public DependencyConsumer, public FDAlgorithm {
private:
    std::list<std::shared_ptr<SearchSpace>> searchSpaces_;
    //config, uccConsumer, fdConsumer,

    CachingMethod cachingMethod_;
    CacheEvictionMethod evictionMethod_;
    double cachingMethodValue;

    Configuration configuration_;
public:
    //explicit Pyro(fs::path const& path, char separator, bool hasHeader);
    explicit Pyro(fs::path const& path, char separator = ',', bool hasHeader = true,
                  int seed = 0, double maxError = 0.01, unsigned int maxLHS = -1);

    unsigned long long execute() override;
};
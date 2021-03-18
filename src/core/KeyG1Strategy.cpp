#include <unordered_map>
#include "KeyG1Strategy.h"
#include "SearchSpace.h"
#include "PLICache.h"

double KeyG1Strategy::calculateKeyError(PositionListIndex* pli) const {
    return calculateKeyError(pli->getNepAsLong());
}

double KeyG1Strategy::calculateKeyError(double numViolatingTuplePairs) const {
    unsigned long long numTuplePairs = context_->getColumnLayoutRelationData()->getNumTuplePairs();
    if (numTuplePairs == 0) return 0;
    double g1 = numViolatingTuplePairs / numTuplePairs;
    return round(g1);
}

void KeyG1Strategy::ensureInitialized(SearchSpace* searchSpace) const {
    if (searchSpace->isInitialized_) return;

    for (auto& column : context_->getSchema()->getColumns()) {
        if (DependencyStrategy::isIrrelevantColumn(column->getIndex())) continue;

        searchSpace->addLaunchPad(createDependencyCandidate(static_cast<Vertical>(*column)));
    }

    searchSpace->isInitialized_ = true;
}

double KeyG1Strategy::calculateError(Vertical const& keyCandidate) const {
    auto pli = context_->getPLICache()->getOrCreateFor(keyCandidate, context_);
    double error = calculateKeyError(pli);
    calcCount_++;
    return error;
}

ConfidenceInterval KeyG1Strategy::calculateKeyError(ConfidenceInterval const& estimatedQualityPairs) const {
    return ConfidenceInterval(calculateKeyError(estimatedQualityPairs.getMin()),
            calculateKeyError(estimatedQualityPairs.getMean()),
            calculateKeyError(estimatedQualityPairs.getMax()));
}

DependencyCandidate KeyG1Strategy::createDependencyCandidate(Vertical const& vertical) const {
    if (vertical.getArity() == 1) {
        auto pli = context_->getPLICache()->getOrCreateFor(vertical, context_);
        double keyError = calculateKeyError(pli->getNepAsLong());
        return DependencyCandidate(vertical, ConfidenceInterval(keyError), true);
    }

    if (context_->agreeSetSamples_ == nullptr) {
        return DependencyCandidate(vertical, ConfidenceInterval(0, .5, 1), false);
    }

    std::shared_ptr<AgreeSetSample> agreeSetSample = context_->getAgreeSetSample(vertical);
    ConfidenceInterval estimatedEqualityPairs = agreeSetSample
            ->estimateAgreements(vertical, context_->getConfiiguration().estimateConfidence)
            .multiply(context_->getColumnLayoutRelationData()->getNumTuplePairs());
    ConfidenceInterval keyError = calculateKeyError(estimatedEqualityPairs);
    return DependencyCandidate(vertical, keyError, false);
}

void KeyG1Strategy::registerDependency(Vertical const& vertical, double error,
                                      DependencyConsumer const& discoveryUnit) const {
    discoveryUnit.registerUcc(vertical, error, 0); // TODO: calculate score?
}

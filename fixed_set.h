#include <iostream>
#include <vector>
#include <random>
#include <optional>

using std::vector;
using std::int64_t;

class HashFunction {
public:
    HashFunction() = default;

    HashFunction(int slope, int constant_term, int64_t prime) :
        slope_(slope), constant_term_(constant_term), prime_(prime) {};

    int GetHash(int64_t value) const {
        int hash_value = (value * slope_ + constant_term_) % prime_;
        if (hash_value < 0) {
            hash_value += prime_;
        }
        return hash_value;
    }

private:
    int64_t slope_;
    int64_t constant_term_;
    int64_t prime_;
};

HashFunction GetHashFunction(const int64_t prime, std::mt19937& generator) {
    std::uniform_int_distribution<int> slope_distribution(1, prime - 1);
    std::uniform_int_distribution<int> constant_term_distribution(0, prime - 1);
    int slope = slope_distribution(generator);
    int constant_term = constant_term_distribution(generator);

    return HashFunction(slope, constant_term, prime);
}

int GetSumOfSquares(const vector<int>& arr) {
    return std::inner_product(arr.begin(), arr.end(), arr.begin(), 0);
}

vector<int> GetDistributionByBuckets(const vector<int>& numbers,
                          int bucket_num,
                          const HashFunction& hash_function) {
    vector<int> distribution(bucket_num, 0);

    for (const auto& num : numbers) {
        ++distribution[hash_function.GetHash(num) % bucket_num];
    }

    return distribution;
}

template <typename F>
HashFunction GetHashFunctionSatisfyingPredicate(const vector<int>& numbers,
                                            int bucket_num,
                                            const F& predicate) {
    const int64_t prime = 2147483053;
    static std::random_device dev;
    static std::mt19937 generator(dev());

    HashFunction hash_function;
    vector<int> distribution_by_buckets;
    do {
        hash_function = GetHashFunction(prime, generator);
        distribution_by_buckets = GetDistributionByBuckets(
                numbers, bucket_num, hash_function);
    } while (!predicate(distribution_by_buckets));

    return hash_function;
}

class InnerHashTable {
public:
    InnerHashTable() = default;

    void Initialize(const vector<int>& numbers) {
        if (numbers.empty()) {
            return;
        }

        int elements_num = numbers.size();
        int square_of_elements_num = elements_num * elements_num;
        table_.clear();
        table_.resize(square_of_elements_num);

        auto predicate = [&](const vector<int>& numbers) {
            return GetSumOfSquares(numbers) <= elements_num;
        };
        hash_function_ = GetHashFunctionSatisfyingPredicate(numbers,
                                                            square_of_elements_num,
                                                            predicate);

        for (const auto& number : numbers) {
            table_[hash_function_.GetHash(number) % square_of_elements_num] = number;
        }
    }

    bool Contains(int64_t number) const {
        if (table_.empty()) {
            return false;
        }

        int id = hash_function_.GetHash(number) % table_.size();
        return table_[id].has_value() && (table_[id] == number);
    }

private:
    vector<std::optional<int>> table_;
    HashFunction hash_function_;
};

class FixedSet {
public:
    FixedSet() = default;
    void Initialize(const vector<int>& numbers);
    bool Contains(int number) const;

private:
    vector<InnerHashTable> table_;
    HashFunction first_level_;
};

void FixedSet::Initialize(const vector<int> &numbers) {
    if (numbers.empty()) {
        return;
    }

    int elements_num = numbers.size();
    auto predicate = [&](const vector<int>& numbers) {
        return GetSumOfSquares(numbers) <= 4 * elements_num;
    };
    first_level_ = GetHashFunctionSatisfyingPredicate(numbers,
                                                      elements_num,
                                                        predicate);

    vector<vector<int>> table_basic(elements_num);
    for (const auto& num : numbers) {
        table_basic[first_level_.GetHash(num) % elements_num].push_back(num);
    }

    table_.clear();
    table_.resize(elements_num);
    for (int hash_value = 0; hash_value < elements_num; ++hash_value) {
        table_[hash_value].Initialize(table_basic[hash_value]);
    }
}

bool FixedSet::Contains(int number) const {
    if (table_.empty()) {
        return false;
    }

    int id = first_level_.GetHash(number) % table_.size();
    return table_[id].Contains(number);
}

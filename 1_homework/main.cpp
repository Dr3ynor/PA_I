#include <iostream>
#include <vector>
#include <limits>
#include <mutex>
#include <thread>
#include <future>
#include <algorithm>

std::mutex mtx; // Mutex for shared resources

struct Facility {
    int width;
    int index;
};

class SRFLP {
private:
    std::vector<Facility> facilities;
    std::vector<std::vector<int>> weights;
    int best_cost;

public:
    SRFLP(const std::vector<Facility>& fac, const std::vector<std::vector<int>>& w)
        : facilities(fac), weights(w), best_cost(std::numeric_limits<int>::max()) {}

    int calculateCost(const std::vector<Facility>& layout) {
        int cost = 0;
        for (size_t i = 0; i < layout.size(); ++i) {
            for (size_t j = i + 1; j < layout.size(); ++j) {
                int distance = abs(layout[j].index - layout[i].index);
                cost += weights[layout[i].index][layout[j].index] * distance;
            }
        }
        return cost;
    }

    void branchAndBound(std::vector<Facility> currentLayout, int currentCost) {
        if (currentLayout.size() == facilities.size()) {
            std::lock_guard<std::mutex> lock(mtx);
            if (currentCost < best_cost) {
                best_cost = currentCost;
            }
            return;
        }

        for (const Facility& f : facilities) {
            if (std::find(currentLayout.begin(), currentLayout.end(), f) == currentLayout.end()) {
                currentLayout.push_back(f);
                int cost = calculateCost(currentLayout);

                if (cost < best_cost) {
                    branchAndBound(currentLayout, cost);
                }
                currentLayout.pop_back();
            }
        }
    }

    void solve() {
        std::vector<std::future<void>> futures;
        for (int i = 0; i < facilities.size(); ++i) {
            futures.emplace_back(std::async(std::launch::async, &SRFLP::branchAndBound, this, std::vector<Facility>{facilities[i]}, 0));
        }

        for (auto& f : futures) {
            f.get();
        }

        std::cout << "Best cost: " << best_cost << std::endl;
    }
};

int main() {
    std::vector<Facility> facilities = { {5, 0}, {3, 1}, {4, 2} };
    std::vector<std::vector<int>> weights = {
        {0, 3, 1},
        {3, 0, 2},
        {1, 2, 0}
    };

    SRFLP srflp(facilities, weights);
    srflp.solve();

    return 0;
}

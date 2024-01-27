#include "iostream"
#include "algorithm"
#include "vector"
#include <iterator>

using std::int64_t;
using std::vector;

struct Player {
    int64_t efficiency;
    int64_t index;
};

struct Team {
    std::vector<Player>::iterator left;
    std::vector<Player>::iterator right;
    int64_t team_efficiency;
};

struct ComparePlayersByEfficiency {
    bool operator() (const Player& first, const Player& second) {
        return first.efficiency <= second.efficiency;
    }
};

struct ComparePlayersByIndex {
    bool operator() (const Player& first, const Player& second) {
        return first.index <= second.index;
    }
};

template<typename It, typename Comparator>
void Merge(It begin, It mid, It end, Comparator comp,
           std::vector<typename It::value_type>& buffer) {
    buffer.reserve(std::distance(begin, end));

    It left_it{begin};
    It right_it{mid};
    const It mid_it{mid};
    const It end_it{end};

    int64_t ind = 0;
    while (left_it != mid_it && right_it != end_it) {
        if (comp(*left_it, *right_it)) {
            buffer[ind] = *left_it;
            ++ind;
            ++left_it;
        } else {
            buffer[ind] = *right_it;
            ++ind;
            ++right_it;
        }
    }

    while (left_it != mid_it) {
        buffer[ind] = *left_it;
        ++ind;
        ++left_it;
    }

    while (right_it != end_it) {
        buffer[ind] = *right_it;
        ++ind;
        ++right_it;
    }

    std::move(buffer.cbegin(), buffer.cbegin() + ind, begin);
    buffer.clear();
}

template<typename It, typename Comparator>
void MergeSortImplementation(It begin, It end, Comparator comp,
                             std::vector<typename It::value_type>& buffer) {
    auto size = std::distance(begin, end);
    if (size <= 1) {
        return;
    }

    auto mid = std::next(begin, size / 2);
    MergeSortImplementation(begin, mid, comp, buffer);
    MergeSortImplementation(mid, end, comp, buffer);
    Merge(begin, mid, end, comp, buffer);
}

template<typename It, typename Comparator>
void MergeSort(It begin, It end, Comparator comp) {
    std::vector<typename It::value_type> buffer;
    MergeSortImplementation(begin, end, comp, buffer);
}

bool IsFirstTeamBetterThanSecond(const Team& first, const Team& second) {
    return first.team_efficiency > second.team_efficiency;
}

vector<Player> BuildMostEffectiveSolidaryTeam(vector<Player> players) {
    if (players.size() <= 2) {
        return players;
    }

    MergeSort(players.begin(), players.end(), ComparePlayersByEfficiency());

    Team current_team{players.begin(),
                      players.begin() + 1, players[0].efficiency + players[1].efficiency};
    Team best_team = current_team;

    while (current_team.left < players.end()) {
        if (current_team.left == players.end() - 1) {
            current_team.team_efficiency = current_team.left->efficiency;
            if (IsFirstTeamBetterThanSecond(current_team, best_team)) {
                best_team = current_team;
            }
            break;
        }

        if (current_team.left == players.end() - 2) {
            current_team.team_efficiency = current_team.left->efficiency +
                    (current_team.left + 1)->efficiency;
            if (IsFirstTeamBetterThanSecond(current_team, best_team)) {
                best_team = current_team;
            }
            ++current_team.left;
            continue;
        }

        if (current_team.right == players.end() - 1) {
            if (IsFirstTeamBetterThanSecond(current_team, best_team)) {
                best_team = current_team;
            }
            current_team.team_efficiency -= current_team.left->efficiency;
            ++current_team.left;
            continue;
        }

        int64_t first_player_eff = current_team.left->efficiency;
        int64_t second_player_eff = (current_team.left + 1)->efficiency;
        int64_t last_player_eff = (current_team.right + 1)->efficiency;

        if (last_player_eff <= first_player_eff + second_player_eff) {
            current_team.team_efficiency += last_player_eff;
            ++current_team.right;
            continue;
        } else {
            if (IsFirstTeamBetterThanSecond(current_team, best_team)) {
                best_team = current_team;
            }
            ++current_team.left;
            if (current_team.right == current_team.left) {
                current_team.right = current_team.left + 1;
                current_team.team_efficiency = current_team.left->efficiency +
                        current_team.right->efficiency;
            } else {
                current_team.team_efficiency -= first_player_eff;
            }
        }
    }

    return vector<Player>{best_team.left, best_team.right + 1};
}

int64_t CalculateTeamEfficiency(const vector<Player>& team) {
    int64_t sum = 0;
    for (const auto& player : team) {
        sum += player.efficiency;
    }
    return sum;
}

vector<Player> ReadPlayers(std::istream& in) {
    int64_t elements_num;
    in >> elements_num;

    vector<Player> players(elements_num);
    for (int i = 0; i < elements_num; ++i) {
        in >> players[i].efficiency;
        players[i].index = i + 1;
    }
    return players;
}

void PrintPlayers(std::ostream& out, vector<Player>& res) {
    MergeSort(res.begin(), res.end(), ComparePlayersByIndex());
    out << CalculateTeamEfficiency(res) << "\n";

    for (const auto &elem : res) {
        out << elem.index << " ";
    }
    out << "\n";
}

int main() {
    vector<Player> players = ReadPlayers(std::cin);
    vector<Player> res = BuildMostEffectiveSolidaryTeam(players);
    PrintPlayers(std::cout, res);

    return 0;
}

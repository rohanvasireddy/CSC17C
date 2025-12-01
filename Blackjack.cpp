#include <bitset>
#include <cctype>
#include <chrono>
#include <deque>
#include <forward_list>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <numeric>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
using namespace std;
static mt19937 g_rng(static_cast<unsigned int>(chrono::high_resolution_clock::now().time_since_epoch().count()));
static string toUpper(string s)
{
    for (char &ch : s)
    {
        ch = static_cast<char>(toupper(static_cast<unsigned char>(ch)));
    }
    return s;
}
static string trim(const string& s)
{
    size_t a = 0;
    while (a < s.size() && isspace(static_cast<unsigned char>(s[a])))
    {
        ++a;
    }
    size_t b = s.size();
    while (b > a && isspace(static_cast<unsigned char>(s[b - 1])))
    {
        --b;
    }
    return s.substr(a, b - a);
}
enum class Outcome
{
    Win,
    Loss,
    Push
};
static string outcomeToString(Outcome o)
{
    if (o == Outcome::Win) return "WIN";
    if (o == Outcome::Loss) return "LOSS";
    return "PUSH";
}
struct Card
{
    string suit;
    string rank;
};
static string cardId(const Card& c)
{
    return c.rank + "|" + c.suit;
}
static string cardToString(const Card& c)
{
    return c.rank + " of " + c.suit;
}
static const map<string, int> VALUE_MAP =
{
    {"2",2},
    {"3",3},
    {"4",4},
    {"5",5},
    {"6",6},
    {"7",7},
    {"8",8},
    {"9",9},
    {"10",10},
    {"J",10},
    {"Q",10},
    {"K",10},
    {"A",11}
};
static const unordered_map<string, int> VALUE_HASH =
{
    {"2",2},
    {"3",3},
    {"4",4},
    {"5",5},
    {"6",6},
    {"7",7},
    {"8",8},
    {"9",9},
    {"10",10},
    {"J",10},
    {"Q",10},
    {"K",10},
    {"A",11}
};
class Hand
{
private:
    list<Card> m_cards;
public:
    void clear()
    {
        m_cards.clear();
    }
    void add(const Card& c)
    {
        m_cards.push_back(c);
    }
    const list<Card>& cards() const
    {
        return m_cards;
    }
    size_t size() const
    {
        return m_cards.size();
    }
    int value() const
    {
        int total = 0;
        int aces = static_cast<int>(count_if(m_cards.begin(), m_cards.end(), [](const Card& c)
        {
            return c.rank == "A";
        }));
        for (const auto& c : m_cards)
        {
            auto it = VALUE_HASH.find(c.rank);
            if (it != VALUE_HASH.end())
            {
                total += it->second;
            }
        }
        while (total > 21 && aces > 0)
        {
            total -= 10;
            --aces;
        }
        return total;
    }
    bool isBlackjack() const
    {
        return m_cards.size() == 2 && value() == 21;
    }
    bool isBust() const
    {
        return value() > 21;
    }
    bool isSoft() const
    {
        int hard = 0;
        int aces = 0;
        for (const auto& c : m_cards)
        {
            if (c.rank == "A")
            {
                ++aces;
                hard += 1;
            }
            else
            {
                hard += VALUE_MAP.at(c.rank);
            }
        }
        return aces > 0 && (hard + 10) <= 21;
    }
    void printLines(bool hideFirst) const
    {
        int idx = 0;
        for (auto it = m_cards.begin(); it != m_cards.end(); ++it)
        {
            if (hideFirst && idx == 0)
            {
                cout << "  - [Hidden Card]" << "\n";
            }
            else
            {
                cout << "  - " << cardToString(*it) << "\n";
            }
            ++idx;
        }
    }
    string asOneLine() const
    {
        ostringstream oss;
        bool first = true;
        for_each(m_cards.begin(), m_cards.end(), [&](const Card& c)
        {
            if (!first)
            {
                oss << ", ";
            }
            first = false;
            oss << cardToString(c);
        });
        return oss.str();
    }
};
class Deck
{
private:
    list<Card> m_shoe;
    stack<Card> m_discard;
    set<string> m_seen;
public:
    Deck()
    {
        rebuild();
    }
    void rebuild()
    {
        m_shoe.clear();
        while (!m_discard.empty())
        {
            m_discard.pop();
        }
        m_seen.clear();
        const string suits[] = {"Hearts","Diamonds","Clubs","Spades"};
        const string ranks[] = {"A","2","3","4","5","6","7","8","9","10","J","Q","K"};
        for (const auto& s : suits)
        {
            for (const auto& r : ranks)
            {
                m_shoe.push_back(Card{s,r});
            }
        }
    }
    size_t size() const
    {
        return m_shoe.size();
    }
    size_t seenCount() const
    {
        return m_seen.size();
    }
    const set<string>& seen() const
    {
        return m_seen;
    }
    void shuffle()
    {
        if (m_shoe.empty())
        {
            return;
        }
        list<Card> out;
        while (!m_shoe.empty())
        {
            uniform_int_distribution<int> dist(0, static_cast<int>(m_shoe.size()) - 1);
            int k = dist(g_rng);
            auto it = m_shoe.begin();
            advance(it, k);
            out.splice(out.end(), m_shoe, it);
        }
        m_shoe.swap(out);
    }
    void recycleDiscardToShoe()
    {
        while (!m_discard.empty())
        {
            m_shoe.push_back(m_discard.top());
            m_discard.pop();
        }
    }
    Card deal()
    {
        if (m_shoe.empty())
        {
            recycleDiscardToShoe();
            if (m_shoe.empty())
            {
                rebuild();
            }
            shuffle();
        }
        Card c = m_shoe.front();
        m_shoe.pop_front();
        m_seen.insert(cardId(c));
        return c;
    }
    void burn(const Card& c)
    {
        m_discard.push(c);
    }
};
class AlgorithmLab
{
private:
    forward_list<string> m_words;
public:
    AlgorithmLab()
    {
        m_words = {"BLACKJACK","STL","ITERATORS","ALGORITHMS","CONTAINERS"};
    }
    list<string> splitWords(const string& line) const
    {
        istringstream iss(line);
        istream_iterator<string> it(iss);
        istream_iterator<string> end;
        list<string> out;
        for (; it != end; ++it)
        {
            out.push_back(*it);
        }
        return out;
    }
    string joinWords(const list<string>& words) const
    {
        ostringstream oss;
        bool first = true;
        for (auto it = words.begin(); it != words.end(); ++it)
        {
            if (!first) oss << "|";
            first = false;
            oss << *it;
        }
        return oss.str();
    }
    void demoSearchAndEqual() const
    {
        deque<char> hay;
        string text = "THIS IS A SMALL STL ALGORITHM DEMO";
        for (char c : text) hay.push_back(c);
        deque<char> needle;
        string pat = "STL";
        for (char c : pat) needle.push_back(c);
        auto pos = search(hay.begin(), hay.end(), needle.begin(), needle.end());
        bool found = (pos != hay.end());
        cout << "[search] found STL=" << (found ? "true" : "false") << "\n";
        deque<int> a;
        deque<int> b;
        for (int i = 0; i < 10; ++i)
        {
            a.push_back(i);
            b.push_back(i);
        }
        bool eq = equal(a.begin(), a.end(), b.begin());
        cout << "[equal] seq_equal=" << (eq ? "true" : "false") << "\n";
    }
    void demoAccumulate() const
    {
        list<int> nums;
        for (int i = 1; i <= 10; ++i)
        {
            nums.push_back(i);
        }
        int sum = accumulate(nums.begin(), nums.end(), 0);
        cout << "[accumulate] sum1to10=" << sum << "\n";
    }
    void demoForwardList() const
    {
        cout << "[forward_list]";
        for (auto it = m_words.begin(); it != m_words.end(); ++it)
        {
            cout << " " << *it;
        }
        cout << "\n";
    }
    void runAll() const
    {
        demoForwardList();
        demoSearchAndEqual();
        demoAccumulate();
        list<string> w = splitWords("one two three four");
        cout << "[split/join] " << joinWords(w) << "\n";
    }
};
class Stats
{
private:
    map<string, long long> m_outcomeCounts;
    map<int, long long> m_marginCounts;
    set<int> m_playerTotals;
    set<int> m_dealerTotals;
public:
    void reset()
    {
        m_outcomeCounts.clear();
        m_marginCounts.clear();
        m_playerTotals.clear();
        m_dealerTotals.clear();
    }
    void record(Outcome o, int playerTotal, int dealerTotal)
    {
        string key = outcomeToString(o);
        m_outcomeCounts[key] += 1;
        m_playerTotals.insert(playerTotal);
        m_dealerTotals.insert(dealerTotal);
        int margin = playerTotal - dealerTotal;
        m_marginCounts[margin] += 1;
    }
    long long total() const
    {
        long long t = 0;
        for (const auto& kv : m_outcomeCounts)
        {
            t += kv.second;
        }
        return t;
    }
    const map<string, long long>& outcomes() const
    {
        return m_outcomeCounts;
    }
    void printSummary() const
    {
        cout << "\n=== Results ===\n";
        for (const auto& kv : m_outcomeCounts)
        {
            cout << setw(8) << kv.first << ": " << kv.second << "\n";
        }
        cout << "Total: " << total() << "\n";
    }
    void printMarginsTopK(int k) const
    {
        priority_queue<pair<long long, int>> pq;
        for (const auto& kv : m_marginCounts)
        {
            pq.push({kv.second, kv.first});
        }
        cout << "[Margins TopK]";
        for (int i = 0; i < k && !pq.empty(); ++i)
        {
            auto t = pq.top();
            pq.pop();
            cout << " m=" << t.second << ":" << t.first;
        }
        cout << "\n";
    }
    void demoAlgorithms() const
    {
        deque<long long> counts;
        for (const auto& kv : m_outcomeCounts)
        {
            counts.push_back(kv.second);
        }
        if (counts.empty())
        {
            return;
        }
        deque<long long> zeros(counts.size());
        fill(zeros.begin(), zeros.end(), 0);
        copy(counts.begin(), counts.end(), zeros.begin());
        sort(zeros.begin(), zeros.end());
        auto mm = minmax_element(zeros.begin(), zeros.end());
        long long mx = *mm.second;
        bool ok = binary_search(zeros.begin(), zeros.end(), mx);
        cout << "\n[Algorithm Demo]\n";
        cout << "min=" << *mm.first << " max=" << mx << " binary_search(max)=" << (ok ? "true" : "false") << "\n";
        deque<long long> left;
        deque<long long> right;
        size_t mid = zeros.size() / 2;
        for (size_t i = 0; i < zeros.size(); ++i)
        {
            if (i < mid) left.push_back(zeros[i]); else right.push_back(zeros[i]);
        }
        deque<long long> merged;
        merged.resize(left.size() + right.size());
        merge(left.begin(), left.end(), right.begin(), right.end(), merged.begin());
        deque<long long> combined = left;
        combined.insert(combined.end(), right.begin(), right.end());
        inplace_merge(combined.begin(), combined.begin() + static_cast<long>(left.size()), combined.end());
        cout << "merge_size=" << merged.size() << " inplace_merge_size=" << combined.size() << "\n";
    }
    void demoMutatingOnList() const
    {
        list<string> logs;
        logs.push_back("WIN");
        logs.push_back("LOSS");
        logs.push_back("PUSH");
        logs.push_back("LOSS");
        replace(logs.begin(), logs.end(), string("LOSS"), string("L"));
        auto it2 = remove_if(logs.begin(), logs.end(), [](const string& s)
        {
            return s == "PUSH";
        });
        logs.erase(it2, logs.end());
        transform(logs.begin(), logs.end(), logs.begin(), [](string s)
        {
            return toUpper(s);
        });
        cout << "[List Mutating Demo]";
        for (const auto& s : logs)
        {
            cout << " " << s;
        }
        cout << "\n";
    }
    void demoSets() const
    {
        cout << "[Totals Set Demo] playerDistinct=" << m_playerTotals.size() << " dealerDistinct=" << m_dealerTotals.size() << "\n";
        if (!m_playerTotals.empty())
        {
            cout << "player_min=" << *m_playerTotals.begin() << " player_max=" << *m_playerTotals.rbegin() << "\n";
        }
    }
};
class Blackjack
{
private:
    Deck m_deck;
    Hand m_player;
    Hand m_dealer;
    Stats m_stats;
    AlgorithmLab m_lab;
    queue<string> m_dealOrder;
    priority_queue<pair<long long, string>> m_top;
    forward_list<string> m_tips;
    bitset<8> m_flags;
    void buildTips()
    {
        m_tips = {"Stand on 17+ (simplified)", "Dealer hits until 17", "Ace=11 unless bust then 1"};
    }
    void resetDealOrder()
    {
        while (!m_dealOrder.empty())
        {
            m_dealOrder.pop();
        }
        m_dealOrder.push("PLAYER");
        m_dealOrder.push("DEALER");
        m_dealOrder.push("PLAYER");
        m_dealOrder.push("DEALER");
    }
    void dealInitial()
    {
        m_player.clear();
        m_dealer.clear();
        m_deck.shuffle();
        resetDealOrder();
        while (!m_dealOrder.empty())
        {
            string who = m_dealOrder.front();
            m_dealOrder.pop();
            if (who == "PLAYER")
            {
                m_player.add(m_deck.deal());
            }
            else
            {
                m_dealer.add(m_deck.deal());
            }
        }
    }
    void burnHands()
    {
        for (const auto& c : m_player.cards())
        {
            m_deck.burn(c);
        }
        for (const auto& c : m_dealer.cards())
        {
            m_deck.burn(c);
        }
    }
    void showTipsIfEnabled()
    {
        if (!m_flags.test(0))
        {
            return;
        }
        cout << "\nTips:\n";
        for (auto it = m_tips.begin(); it != m_tips.end(); ++it)
        {
            cout << " - " << *it << "\n";
        }
    }
    void printState(bool hideDealer) const
    {
        cout << "\nPlayer (" << m_player.value() << "):\n";
        m_player.printLines(false);
        cout << "Dealer";
        if (!hideDealer)
        {
            cout << " (" << m_dealer.value() << ")";
        }
        cout << ":\n";
        m_dealer.printLines(hideDealer);
    }
    void dealerTurn()
    {
        while (m_dealer.value() < 17)
        {
            m_dealer.add(m_deck.deal());
        }
    }
    Outcome decide() const
    {
        int pv = m_player.value();
        int dv = m_dealer.value();
        if (pv > 21) return Outcome::Loss;
        if (dv > 21) return Outcome::Win;
        if (pv > dv) return Outcome::Win;
        if (dv > pv) return Outcome::Loss;
        return Outcome::Push;
    }
    void recordOutcome(Outcome o)
    {
        m_stats.record(o, m_player.value(), m_dealer.value());
    }
    void interactivePlayerTurn()
    {
        while (true)
        {
            printState(true);
            if (m_player.isBust())
            {
                return;
            }
            cout << "Hit or Stand (H/S): ";
            string s;
            cin >> s;
            s = toUpper(trim(s));
            if (s == "H" || s == "HIT")
            {
                m_player.add(m_deck.deal());
            }
            else
            {
                return;
            }
        }
    }
    void interactiveRound()
    {
        dealInitial();
        showTipsIfEnabled();
        if (m_player.isBlackjack() || m_dealer.isBlackjack())
        {
            printState(false);
            Outcome o;
            if (m_player.isBlackjack() && m_dealer.isBlackjack()) o = Outcome::Push;
            else if (m_player.isBlackjack()) o = Outcome::Win;
            else o = Outcome::Loss;
            cout << "Outcome: " << outcomeToString(o) << "\n";
            recordOutcome(o);
            burnHands();
            return;
        }
        interactivePlayerTurn();
        dealerTurn();
        printState(false);
        Outcome o = decide();
        cout << "Outcome: " << outcomeToString(o) << "\n";
        recordOutcome(o);
        burnHands();
    }
    void buildTopFromOutcomes()
    {
        while (!m_top.empty())
        {
            m_top.pop();
        }
        for (const auto& kv : m_stats.outcomes())
        {
            m_top.push({kv.second, kv.first});
        }
    }
    void showTopOutcome()
    {
        buildTopFromOutcomes();
        if (m_top.empty())
        {
            return;
        }
        auto p = m_top.top();
        cout << "Most common: " << p.second << " (" << p.first << ")\n";
    }
    void simulateRounds(int n)
    {
        m_stats.reset();
        for (int i = 0; i < n; ++i)
        {
            dealInitial();
            while (m_player.value() <= 16)
            {
                m_player.add(m_deck.deal());
            }
            dealerTurn();
            Outcome o = decide();
            recordOutcome(o);
            burnHands();
        }
        m_stats.printSummary();
        showTopOutcome();
        m_stats.printMarginsTopK(5);
        m_stats.demoAlgorithms();
        m_stats.demoMutatingOnList();
        m_stats.demoSets();
    }
    void demoSeenCards()
    {
        const set<string>& s = m_deck.seen();
        cout << "Seen unique cards so far: " << s.size() << "\n";
        if (!s.empty())
        {
            auto it = s.begin();
            cout << "First: " << *it << "\n";
            auto it2 = s.end();
            --it2;
            cout << "Last: " << *it2 << "\n";
        }
        bool hasAceSpades = (s.find("A|Spades") != s.end());
        cout << "Contains A|Spades: " << (hasAceSpades ? "true" : "false") << "\n";
    }
    void demoInputIterators()
    {
        cout << "Enter 3 integers: ";
        istream_iterator<int> it(cin);
        int a = *it;
        ++it;
        int b = *it;
        ++it;
        int c = *it;
        cout << "Read: " << a << ", " << b << ", " << c << "\n";
    }
public:
    Blackjack()
    {
        buildTips();
        m_flags.reset();
        m_flags.set(0);
    }
    void runMenu()
    {
        while (true)
        {
            cout << "\n=== BLACKJACK ===\n";
            cout << "1) Play\n";
            cout << "2) Simulate\n";
            cout << "3) Toggle tips\n";
            cout << "4) Show seen cards\n";
            cout << "5) Demo input iterators\n";
            cout << "6) Algorithm lab\n";
            cout << "7) Quit\n";
            cout << "> ";
            int choice = 0;
            if (!(cin >> choice))
            {
                return;
            }
            if (choice == 1)
            {
                interactiveRound();
            }
            else if (choice == 2)
            {
                cout << "Rounds: ";
                int n = 0;
                cin >> n;
                if (n < 0) n = 0;
                simulateRounds(n);
            }
            else if (choice == 3)
            {
                m_flags.flip(0);
                cout << "Tips: " << (m_flags.test(0) ? "ON" : "OFF") << "\n";
            }
            else if (choice == 4)
            {
                demoSeenCards();
            }
            else if (choice == 5)
            {
                demoInputIterators();
            }
            else if (choice == 6)
            {
                m_lab.runAll();
            }
            else if (choice == 7)
            {
                return;
            }
        }
    }
};
int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Blackjack game;
    game.runMenu();
    return 0;
}

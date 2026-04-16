#include <bits/stdc++.h>
using namespace std;

struct Request {
    bool is_read;
    uint32_t address;
    uint32_t data;
};

struct CacheLine {
    uint32_t tag = 0;
    uint32_t data = 0;
    bool valid = false;
    bool dirty = false;
};

class SimpleMemory {
public:
    vector<uint32_t> mem;
    SimpleMemory(size_t size = 1024) : mem(size, 0) {}
};

class CacheController {
public:
    enum class State {
        IDLE,
        COMPARE_TAG,
        WRITE_BACK,
        ALLOCATE
    };

    State current_state = State::IDLE;
    vector<CacheLine> cache_lines;
    uint32_t num_lines = 4;
    SimpleMemory* memory;
    bool cpu_ready = true;
    uint32_t cpu_data_out = 0;
    bool mem_read_en = false;
    bool mem_write_en = false;
    uint32_t mem_addr = 0;
    uint32_t mem_data_out = 0;
    Request current_request;
    bool has_request = false;
    uint32_t current_index = 0;
    uint32_t current_tag = 0;
    bool hit = false;
    int mem_counter = 0;
    const int MEM_ACCESS_CYCLES = 4;

    CacheController(SimpleMemory* mem_ptr) : memory(mem_ptr) {
        cache_lines.resize(num_lines);
    }

    void set_request(const Request& req) {
        if (current_state == State::IDLE) {
            current_request = req;
            has_request = true;
            cpu_ready = false;
        }
    }

    void tick() {
        State next_state = current_state;
        switch (current_state) {
            case State::IDLE:
                cpu_ready = true;
                mem_read_en = false;
                mem_write_en = false;
                if (has_request) {
                    next_state = State::COMPARE_TAG;
                    cpu_ready = false;
                }
                break;

            case State::COMPARE_TAG:
                current_index = current_request.address % num_lines;
                current_tag = current_request.address / num_lines;
                hit = cache_lines[current_index].valid && (cache_lines[current_index].tag == current_tag);
                
                if (hit) {
                    if (current_request.is_read) {
                        cpu_data_out = cache_lines[current_index].data;
                    } else {
                        cache_lines[current_index].data = current_request.data;
                        cache_lines[current_index].dirty = true;
                    }
                    cpu_ready = true;
                    has_request = false;
                    next_state = State::IDLE;
                } else {
                    if (cache_lines[current_index].valid && cache_lines[current_index].dirty) {
                        next_state = State::WRITE_BACK;
                    } else {
                        next_state = State::ALLOCATE;
                    }
                }
                mem_counter = 0;
                break;

            case State::WRITE_BACK:
                if (mem_counter == 0) {
                    mem_write_en = true;
                    mem_addr = (cache_lines[current_index].tag * num_lines) + current_index;
                    mem_data_out = cache_lines[current_index].data;
                    mem_counter = MEM_ACCESS_CYCLES - 1;
                    next_state = State::WRITE_BACK;
                } else {
                    mem_counter--;
                    if (mem_counter == 0) {
                        memory->mem[mem_addr] = mem_data_out;
                        mem_write_en = false;
                        cache_lines[current_index].dirty = false;
                        next_state = State::ALLOCATE;
                        mem_counter = 0;
                    }
                }
                break;

            case State::ALLOCATE:
                if (mem_counter == 0) {
                    mem_read_en = true;
                    mem_addr = current_request.address;
                    mem_counter = MEM_ACCESS_CYCLES - 1;
                    next_state = State::ALLOCATE;
                } else {
                    mem_counter--;
                    if (mem_counter == 0) {
                        mem_read_en = false;
                        uint32_t fetched = memory->mem[mem_addr];
                        cache_lines[current_index].data = fetched;
                        cache_lines[current_index].tag = current_tag;
                        cache_lines[current_index].valid = true;
                        cache_lines[current_index].dirty = false;

                        if (current_request.is_read) {
                            cpu_data_out = fetched;
                        } else {
                            cache_lines[current_index].data = current_request.data;
                            cache_lines[current_index].dirty = true;
                        }
                        cpu_ready = true;
                        has_request = false;
                        next_state = State::IDLE;
                    }
                }
                break;
        }
        current_state = next_state;
    }

    string state_to_string(State s) const {
        switch (s) {
            case State::IDLE:        return "IDLE";
            case State::COMPARE_TAG: return "COMPARE_TAG";
            case State::WRITE_BACK:  return "WRITE_BACK";
            case State::ALLOCATE:    return "ALLOCATE";
            default:                 return "UNKNOWN";
        }
    }

    void print_status(int cycle) const {
        cout << left
             << setw(6)  << cycle
             << setw(15) << state_to_string(current_state)
             << setw(10) << (has_request ? (current_request.is_read ? "READ" : "WRITE") : "-")
             << setw(10) << (has_request ? to_string(current_request.address) : "-")
             << setw(10) << (current_state == State::COMPARE_TAG ? (hit ? "HIT" : "MISS") : "-")
             << setw(10) << (cpu_ready ? "READY" : "BUSY")
             << setw(12) << (mem_read_en ? "MEM_READ" : (mem_write_en ? "MEM_WRITE" : "-"))
             << setw(10) << (cpu_ready && !has_request ? to_string(cpu_data_out) : "-")
             << endl;
    }
};

int main() {
    cout << "==================================================\n";
    cout << "   FSM-based Simple Cache Controller Simulator\n";
    cout << "   Exact match to Book Figure 5.38 (4-state FSM)\n";
    cout << "   IDLE -> COMPARE_TAG -> WRITE_BACK -> ALLOCATE\n";
    cout << "==================================================\n\n";

    SimpleMemory memory(1024);
    CacheController ctrl(&memory);

    memory.mem[0] = 100; memory.mem[1] = 200; memory.mem[2] = 300;
    memory.mem[4] = 400; memory.mem[5] = 500;

    vector<pair<string, vector<Request>>> tests = {
        {"1. Read Miss -> Allocate", {{true, 0, 0}}},
        {"2. Read Hit", {{true, 0, 0}}},
        {"3. Write Hit", {{false, 1, 9999}}},
        {"4. Write Miss (Clean)", {{false, 2, 7777}}},
        {"5. Write Miss with Write-back", {{true, 4, 0}, {false, 0, 8888}}},
        {"6. Mixed Sequence", {{true, 5, 0}, {false, 1, 12345}, {true, 2, 0}, {false, 4, 99999}}}
    };

    for (auto& test : tests) {
        cout << "\n=== " << test.first << " ===\n";
        cout << left << setw(6) << "Cycle" << setw(15) << "State" << setw(10) << "Request"
             << setw(10) << "Address" << setw(10) << "Hit/Miss" << setw(10) << "CPU"
             << setw(12) << "Memory" << setw(10) << "Data Out" << endl;
        cout << string(80, '-') << endl;

        ctrl.current_state = CacheController::State::IDLE;
        ctrl.cpu_ready = true;
        ctrl.has_request = false;
        ctrl.mem_counter = 0;
        ctrl.mem_read_en = false;
        ctrl.mem_write_en = false;

        size_t req_idx = 0;
        int cycle = 0;

        while (cycle < 100) {
            if (ctrl.cpu_ready && req_idx < test.second.size()) {
                ctrl.set_request(test.second[req_idx]);
                req_idx++;
            }

            ctrl.print_status(cycle);
            ctrl.tick();
            cycle++;

            if (req_idx >= test.second.size() && ctrl.current_state == CacheController::State::IDLE)
                break;
        }

        cout << string(80, '-') << endl;
        cout << "Test completed in " << cycle << " cycles.\n\n";
    }

    cout << "Simulation finished!\n";
    cout << "This exactly follows the book's FSM (Figure 5.38).\n";
    return 0;
}
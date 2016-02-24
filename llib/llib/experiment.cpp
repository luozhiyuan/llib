#include <vector>
#include <cassert>
#include <cstdlib>
#include <cstdint>
//act as a list node

template<typename PODT>
struct PODHandler;

struct Handle {
    Handle():is_freed(true){}
private:
    template<typename T> 
    friend struct PODHandler;
    Handle(std::size_t e, std::size_t n):component_index(e), next_handle_index(n), is_freed(false) {
    }

    std::size_t next_handle_index;
    std::size_t component_index:31;
    std::size_t is_freed:1;

    std::size_t SelfIndex()const {
        assert(!IsFreed());
        return next_handle_index;
    }
    std::size_t NextIndex()const {
        assert(IsFreed());
        return next_handle_index;
    }
    bool IsFreed()const { return is_freed; }
    void SetFree(bool free) { is_freed = free; }

};
static_assert(sizeof(Handle) == 2*sizeof(std::size_t), "");
template<typename PODT>
struct PODHandler {
    Handle Allocate() {

        auto& h = AllocateHandle();
        h.component_index = components_.size();

        component_index_to_handle_index_.push_back(h.SelfIndex());
        components_.push_back(PODT());

        assert(component_index_to_handle_index_.size() == components_.size());
        return h;
    }
    void Deallocate(Handle h) {
        assert(!components_.empty());
        if (h.SelfIndex() >= handles_.size() || handles_[h.SelfIndex()].IsFreed())
            return;
        const auto& handle = handles_[h.SelfIndex()];
        if (handle.component_index < components_.size()) {

           auto old = component_index_to_handle_index_[components_.size() - 1];

            std::swap(components_[handle.component_index], components_.back());
            components_.pop_back();

            component_index_to_handle_index_[handle.component_index] = old;
            component_index_to_handle_index_.pop_back();
            
            handles_[old].component_index = handle.component_index;

            DeallocateHandle(h);
        }
    }

    PODT* Get(Handle h) { return &components_[handles_[h.SelfIndex()].component_index]; }
    bool IsEmpty()const { return components_.empty(); }
    std::size_t Size()const { return components_.size(); }
private:
    std::vector<PODT> components_;
    std::vector<Handle> handles_;
    std::vector<std::size_t> component_index_to_handle_index_;//

    static const std::size_t kNullEntryIndex = -1;
    std::size_t first_free_handle_index_ = kNullEntryIndex;
private:
    Handle& AllocateHandle() {
        if (first_free_handle_index_ == kNullEntryIndex) {
            handles_.push_back(Handle{ components_.size(), handles_.size() });
            return handles_.back();
        }
        auto& r = handles_[first_free_handle_index_];
        auto next_free_handle_index = r.NextIndex();
        r.next_handle_index = first_free_handle_index_; //point to self
        //r.is_freed = false;
        r.SetFree(false);

        first_free_handle_index_ = next_free_handle_index;
        return r;
    }

    void DeallocateHandle(Handle h) {
        auto&  current_free_handle = handles_[h.SelfIndex()];
        current_free_handle.next_handle_index = first_free_handle_index_;
        current_free_handle.SetFree(true);
        first_free_handle_index_ = h.SelfIndex();
    }
};

struct position {
    float x, y, z;
};
void test(){
    PODHandler<position> position_handler;
    std::vector<Handle> handles;
    while (true) {
        assert(position_handler.Size() == handles.size());
        auto h0 = position_handler.Allocate();
        auto p = position_handler.Get(h0);
        p->x = rand();
        p->y = 2;
        p->z = 3;
        handles.push_back(h0);
        srand(rand());
        if (rand() % 29 == 0) {
            position_handler.Deallocate(h0);
            handles.pop_back();
            if (rand() % 3 == 0) {
                while (!handles.empty()){
                    position_handler.Deallocate(handles.back());
                    handles.pop_back();
                }
                assert(position_handler.IsEmpty());
            }
            else if (rand() % 2 == 0) {
                assert(position_handler.Size() == handles.size());
                for (auto x : handles) {
                    position_handler.Deallocate(x);
                }
                assert(position_handler.IsEmpty());
                handles.clear();
            }
        }
    }
}

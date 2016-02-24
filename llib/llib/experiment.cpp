#include <vector>

//act as a list node
struct Handle {
    unsigned int entry_index;
    unsigned int next_entry;
};

template<typename PODT>
struct PODHandler {
    Handle Allocate() {

        auto h = AllocateHandle();
        h.entry_index = components_.size();

        component_index_to_entry_index_.push_back(h.entry_index);
        components_.push_back(PODT());

        assert(component_index_to_entry_index_.size() == components_.size());

    }
    void Deallocate(Handle h) {
        assert(!components_.empty());
        
        if (h.entry_index < components_.size()) {

            auto old = component_index_to_entry_index_[components_.size() - 1];

            std::swap(components_[h.entry_index], components_.back());
            components_.pop_back();

            component_index_to_entry_index_[h.entry_index] = old;
            component_index_to_entry_index_.pop_back();

            DeallocateHandle(h);
        }
    }

    PODT* Get(Handle h) { return &components_[h.entry_index]; }

private:
    std::vector<PODT> components_;
    std::vector<Handle> entries_;
    std::vector<unsigned int> component_index_to_entry_index_;//

    static const unsigned int kNullEntryIndex = -1;
    unsigned int first_free_entry_ = kNullEntryIndex;
private:
    Handle AllocateHandle() {
        if (first_free_entry_ == kNullEntryIndex) {
            entries_.push_back(Handle{ components_.size(), entries_.size() });
            return entries_.back();
        }
        auto r = entries_[first_free_entry_];
        r.next_entry = first_free_entry_; //point to self

        first_free_entry_ = entries_[first_free_entry_].next_entry;
        return r;
    }

    void DeallocateHandle(Handle h) {
        entries_[h.next_entry].next_entry = first_free_entry_;
        first_free_entry_ = h.next_entry;
    }
};
#include <vector>
#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <iostream>

namespace llib {
    namespace experiment {
        //act as a list node
        template<typename PODT, typename>
        struct PODHandler;
        //allocated handle
        struct Handle {
            template<typename T, typename>
            friend struct PODHandler;
            Handle(){}
        protected:
            static const std::size_t kNullHandleIndex = -1;
            std::size_t next_handle_index = kNullHandleIndex;

            explicit Handle(std::size_t n):next_handle_index(n){}

            std::size_t SelfIndex()const {
                assert(!next_handle_index != kNullHandleIndex);
                return next_handle_index;
            }
        };
        template<typename PODT, typename H = Handle>
        struct PODHandler {
        public:
            typedef PODT value_type;
            typedef H handle_type;
        private:
        struct StoredHandle:public handle_type{
            StoredHandle() :is_freed(true) {}

            ///
            StoredHandle(std::size_t e, std::size_t n) :handle_type(n), component_index(e), is_freed(false) {
            }

            std::size_t component_index : 31;
            std::size_t is_freed : 1;

            std::size_t NextIndex()const {
                assert(IsFreed());
                return this->next_handle_index;
            }
            bool IsFreed()const { return is_freed; }
            void SetFree(bool free) { is_freed = free; }

        };
 
        public:
            handle_type Allocate() {

                auto& h = AllocateHandle();
                h.component_index = components_.size();

                component_index_to_handle_index_.push_back(h.SelfIndex());
                components_.push_back(PODT());

                assert(component_index_to_handle_index_.size() == components_.size());
                return h;
            }
            void Deallocate(handle_type h) {
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
                if (IsEmpty()) {
                    //clear handle release handle memory
                    first_free_handle_index_ = kNullEntryIndex;
                    handles_.clear();
                }
            }

            PODT* Get(handle_type h) { return &components_[handles_[h.SelfIndex()].component_index]; }
            bool IsEmpty()const { return components_.empty(); }
            std::size_t Size()const { return components_.size(); }
        private:
            std::vector<PODT> components_;
            std::vector<StoredHandle> handles_;
            std::vector<std::size_t> component_index_to_handle_index_;//

            static const std::size_t kNullEntryIndex = -1;
            std::size_t first_free_handle_index_ = kNullEntryIndex;
        private:
            StoredHandle& AllocateHandle() {
                if (first_free_handle_index_ == kNullEntryIndex) {
                    handles_.push_back(StoredHandle{ components_.size(), handles_.size() });
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

            void DeallocateHandle(handle_type h) {
                auto&  current_free_handle = handles_[h.SelfIndex()];
                current_free_handle.next_handle_index = first_free_handle_index_;
                current_free_handle.SetFree(true);
                first_free_handle_index_ = h.SelfIndex();
            }
        };

        template<typename... Ts>
        struct component_group {
            std::tuple<PODHandler<Ts>...> components;
            //allocate<T>
            //deallocate<T>
            //getcomponent<T>

        };
        template<typename... Ts>
        struct component_handle_group{
            typedef component_group<Ts...> component_group_type;
            typedef component_handle_group<Ts...> type;

            std::array<Handle, sizeof...(Ts)> handles;
        };

        //compile time entity
        template<typename... Ts>
        struct StaticEntity {
            typedef StaticEntity type;
            typedef component_handle_group<Ts...> component_handle_group_type;
            typedef typename component_handle_group<Ts...>::component_group_type component_group_type;
            //std::array<Handle, sizeof...(Ts)> handles;//only a bunch of component handles
            component_handle_group_type handles;
            template<typename C>
            auto GetComponentHandle()const{
                return handles.GetComponentHandle<C>();
            }
            template<typename C>
            auto& GetComponent( World& w) {
                return w.GetComponent<component_group_type, C>(GetComponentHandle<C>());
            }
            template<typename C>
            auto& GetComponent( const World& w)const {
                return w.GetComponent<component_group_type, C>(GetComponentHandle<C>());
            }
 
        };

        struct DynamicEntity {
            //std::vector<DynamicHandle> handles;//can add or remove component
        };

        namespace test {
            struct position {
                float x, y, z;
            };
            void test_case() {
                PODHandler<position> position_handler;
                std::vector<Handle> handles;
                auto niteration = 1000000;
                while (niteration-- > 0) {
                    assert(position_handler.Size() == handles.size());
                    auto h0 = position_handler.Allocate();
                    auto p = position_handler.Get(h0);
                    p->x = rand();
                    p->y = 2;
                    p->z = 3;
                    handles.push_back(h0);
                    srand(rand());
                    if (rand() % 31 == 0) {
                        position_handler.Deallocate(h0);
                        handles.pop_back();
                        if (rand() % 17 == 0) {
                            while (!handles.empty()) {
                                position_handler.Deallocate(handles.back());
                                handles.pop_back();
                            }
                            assert(position_handler.IsEmpty());
                        }
                        else if (rand() % 11 == 0) {
                            assert(position_handler.Size() == handles.size());
                            for (auto x : handles) {
                                position_handler.Deallocate(x);
                            }
                            assert(position_handler.IsEmpty());
                            handles.clear();
                        }
                    }
                }
                std::cout << handles.capacity() << std::endl;
            }
        }
    }
}


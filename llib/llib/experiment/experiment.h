#pragma once


namespace llib {
    namespace experiment {
        namespace ecs {
            template<typename Component>
            struct component_handle {
                unsigned int index;
            };


            template<typename... Component>
            struct entity {
                std::tuple<std::vector<component_handle<Component> >... > handles;
            };
            struct component {};//component group
            struct system {};//system group
            template<typename... ComponentGroups>
            struct world {};

                        class handle_manager {
            };
            template<template<typename ...> Container, typename... Components>
            struct component_group {
                //
            };

            template<typename... ComponentGroup>
            struct component_groups{};

            template<typename Components...>
            auto make_entity(Components&&... args) {
                //return component_group<std::vector, Components...>(std::forward<Components...>(args)...);
            }
            
       }
        namespace test {
            using namespace ecs;
            struct position {
                float x, y, z;
            };
            struct velocity {
                float x, y, z;
            };
            template<typename ... T>
            using component_group_handle = component_group<
                std::vector, T...>;
 
            struct update_position_velocity {
                template<class World> void update(World& w) {
                    using entity = component_group_handle<position, velocity>;
                    w.template update<entity>([](auto& p, auto& v) {
                        static const float acc = 1.f;
                        p.x += v.x;
                        p.y += v.y;
                        p.z += v.z;

                        v.x += acc;
                        v.y += acc;
                        v.z += acc;
                    });
                }
            };
            

            inline void test_ecs() {
                using entity = component_group_handle<position, velocity>;
                using cgs = component_groups<entity>;
                auto w = world<cgs>{};
                auto entity_handle = w.add_entity(make_entity(position{ 3,2,1 }, velocity{1,2,3}));

                auto sg = make_system_group(w, update_position_velocity{});
                sg.update();
                w.remove_entity(entity_handle);
                sg.update();
            }
        }
    }
}

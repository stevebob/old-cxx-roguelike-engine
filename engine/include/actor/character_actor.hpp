#ifndef _CHARACTER_ACTOR_HPP_
#define _CHARACTER_ACTOR_HPP_

#include "world/world.hpp"
#include "observer/observer.hpp"
#include "actor/actor.hpp"
#include "drawing/actor_drawing_interface.hpp"
#include "transaction/transaction.hpp"

class character_actor : public actor, public actor_drawing_interface {
    protected:

    character &character_;
    virtual void act(world &w) = 0;
    virtual bool can_act() const = 0;
    
    observer &observer_;
    std::vector<grid<knowledge_cell>> knowledge_grids_;

    const grid<world_cell> &get_current_grid(world &w) const {
        return w.maps[character_.level_index];
    }
    
    grid<world_cell> &get_current_grid(world &w) {
        return w.maps[character_.level_index];
    }

    grid<knowledge_cell> &get_current_knowledge_grid() {
        return knowledge_grids_[character_.level_index];
    }
    
    const grid<knowledge_cell> &get_current_knowledge_grid() const {
        return knowledge_grids_[character_.level_index];
    }

    void observe_world(world &w) {
        observer_.observe(character_, get_current_grid(w), 
                            this->get_current_knowledge_grid());

        get_current_knowledge_grid().for_each([](knowledge_cell &k) {
            if (k.is_visible()) {
                k.unsee_characters();
            }
        });


        for (typename std::vector<std::unique_ptr<character>>::iterator it = w.characters.begin(); it != w.characters.end(); ++it) {
            if ((*it)->level_index == character_.level_index) {
                knowledge_cell &k = get_current_knowledge_grid().get_cell((*it)->coord);
                if (k.is_visible()) {
                    w.transactions.register_transaction(std::make_unique<see_character_transaction>(this->character_, **it, k));
                }
            }
        }
        w.transactions.process_all(w);
    }

    public:
    character_actor(character &c, world &w, observer &o) : 
        character_(c),
        observer_(o)
    {
        for (typename std::vector<grid<world_cell>>::iterator it = w.maps.begin();
             it != w.maps.end(); ++it) {
            knowledge_grids_.push_back(grid<knowledge_cell>(it->width, it->height));
        }
    }

    void operator()(world& w, callback_registry<world>& cr) {
        if (can_act()) {
            observe_world(w);
            act(w);
            int cooldown = w.transactions.process_all(w);
            if (can_act()) {
                cr.register_callback(*this, cooldown);
            }
        }
    }
    
    const grid<knowledge_cell> &get_knowledge_grid() const {
        return this->get_current_knowledge_grid();
    }
    const character &get_character() const {
        return character_;
    }
};


#endif

/*
 * Copyright (C) 2006-2013 Christopho, Solarus - http://www.solarus-games.org
 * 
 * Solarus is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Solarus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SOLARUS_RANDOM_MOVEMENT_H
#define SOLARUS_RANDOM_MOVEMENT_H

#include "Common.h"
#include "movements/StraightMovement.h"
#include "lowlevel/Rectangle.h"

namespace solarus {

/**
 * \brief A movement that makes successive steps of random directions
 * and (possibly) stays inside a limited rectangle.
 */
class RandomMovement: public StraightMovement {

  public:

    RandomMovement(int speed, int max_radius = 0);
    ~RandomMovement();

    virtual void notify_object_controlled();
    void update();
    void set_suspended(bool suspended);

    int get_max_radius() const;
    void set_max_radius(int max_radius);

    virtual const std::string& get_lua_type_name() const;

  protected:

    virtual void notify_obstacle_reached();

  private:

    int max_radius;                                     /**< if the object goes further than this distance in x or y, it will come back
                                                         * (this is not a hard limit, there is no guarantee) */
    Rectangle bounds;                                   /**< a rectangle the object should not escape from */
    uint32_t next_direction_change_date;                /**< date of the next direction change */

    void set_next_direction();

};

}

#endif


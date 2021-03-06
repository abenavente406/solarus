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
#ifndef SOLARUS_PIXEL_FILTER_H
#define SOLARUS_PIXEL_FILTER_H

#include "Common.h"

namespace solarus {

/**
 * \brief Abstract class for pixel filtering algorithms.
 */
class PixelFilter {

  public:

    PixelFilter();
    virtual ~PixelFilter();

    /**
     * \brief Returns the scaling factor of this algorithm.
     * \return The scaling factor.
     */
    virtual int get_scaling_factor() const = 0;

    /**
     * \brief Applies the algorithm on a rectangle of pixels.
     * \param src The rectangle of pixels in RGBA format.
     * Must be a buffer of size src_width * dst_width.
     * \param src_width Width of the rectangle.
     * \param dst_width Height of the rectangle.
     * \param dst The destination rectangle to write.
     * Must be a buffer of size
     * src_width * dst_width * get_scaling_factor().
     */
    virtual void filter(
        const uint32_t* src,
        int src_width,
        int src_height,
        uint32_t* dst) const = 0;
};

}

#endif


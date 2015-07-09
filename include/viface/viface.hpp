/**
 * Copyright (C) 2015 Hewlett Packard Enterprise Development LP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/**
 * @file viface.hpp
 * Main libviface header file.
 * Define the public interface for libviface.
 */

#ifndef _VIFACE_HPP
#define _VIFACE_HPP

#include <memory>
#include <string>

#include "viface/config.hpp"

#ifdef VIFACE_LIBTINS_SUPPORT
#include "tins/tins.h"
#endif

namespace viface
{
/**
 * @defgroup libviface Public Interface
 * @{
 */

class VIfaceImpl;

class VIface
{
    private:

        std::unique_ptr<VIfaceImpl> pimpl;
        VIface(const VIface& other) = delete;
        VIface& operator=(VIface rhs) = delete;

    public:

        /**
         * Create a VIface object with given name.
         *
         * @param[in]  name Name of the virtual interface.
         */
        VIface(std::string name = "viface%d");
        ~VIface();

        /**
         * Getter method for object associated name.
         *
         * @return the name of the virtual interface.
         */
        std::string getName() const;
};

/** @} */ // End of libviface
};
#endif // _VIFACE_HPP
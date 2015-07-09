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

#ifndef _VIFACE_PRIV_HPP
#define _VIFACE_PRIV_HPP

// Standard
#include <stdexcept>
#include <sstream>

// Third party
// ...

// Framework
#include "viface/viface.hpp"

using namespace std;
using namespace viface;

namespace viface
{
class VIfaceImpl
{
    private:

        string name;

    public:

        VIfaceImpl(string host);

        string getName() const
        {
            return this->name;
        }
};
};
#endif // _VIFACE_PRIV_HPP
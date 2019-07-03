/*
* This source file is part of ARK
* For the latest info, see https://github.com/ArkNX
*
* Copyright (c) 2013-2019 ArkNX authors.
*
* Licensed under the Apache License, Version 2.0 (the "License").
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#pragma once

#include "AFPlatform.hpp"

namespace ark
{

    class AFRandom
    {
    public:
        AFRandom() : seed_(0), get_count_(0)
        {
        }

        ~AFRandom() {}

        //--------------------------------------------------------------------
        //set random seed
        void SetSeed(uint32_t seed)
        {
            seed_ = seed;
            get_count_ = 0;
        }

        uint32_t GetSeed()
        {
            return seed_;
        }

        //--------------------------------------------------------------------
        //generate a random number
        inline uint32_t Random()
        {
            unsigned int next = seed_;
            unsigned int result = 0;

            next *= 1103515245;
            next += 12345;
            result = (unsigned int)(next >> 16) % 2048;

            next *= 1103515245;
            next += 12345;
            result <<= 10;
            result ^= (unsigned int)(next >> 16) % 1024;

            next *= 1103515245;
            next += 12345;
            result <<= 10;
            result ^= (unsigned int)(next >> 16) % 1024;

            seed_ = next;

            get_count_++;

            //reset
            if (get_count_ >= (std::numeric_limits<int>::max() - 1))
            {
                get_count_ = 0;
            }

            return result;
        }

        //--------------------------------------------------------------------
        //generate a random number which ranges in [0, max_range - 1]
        uint32_t Random(uint32_t max_range)
        {
            if (max_range == 0)
            {
                return 0;
            }

            return Random() % max_range;
        }

        //generate a random number which ranges in [min_range, max_range - 1]
        unsigned int Random(uint32_t min_range, uint32_t max_range)
        {
            uint32_t min = 0;
            uint32_t max = 0;

            if (min_range <= max_range)
            {
                min = min_range;
                max = max_range;
            }
            else
            {
                min = max_range;
                max = min_range;
            }

            if (min == max)
            {
                return max;
            }

            return Random(max - min) + min;
        }

        //generate a random number which ranges in [min_range, max_range - 1]
        int Random(int min_range, int max_range)
        {
            if (min_range == max_range)
            {
                return min_range;
            }
            else if (min_range > max_range)
            {
                std::swap(min_range, max_range);
                //int iMinSwap = min_range;
                //min_range = max_range;
                //max_range = iMinSwap;
            }

            int range = max_range - min_range;
            int rand_result = Random((uint32_t)range);

            return rand_result + min_range;
        }

        int GetRandomCount()
        {
            return get_count_;
        }

    private:
        uint32_t seed_;
        int get_count_;
    };

}
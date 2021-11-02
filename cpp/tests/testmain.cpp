/* 
* Copyright (C) 2020-2021 German Aerospace Center (DLR-SC)
*
* Authors: Daniel Abele, Martin Siggel
*
* Contact: Martin J. Kuehn <Martin.Kuehn@DLR.de>
*
* Licensed under the Apache License, Version 2.0 (the "License");
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
*/
#include <epidemiology/utils/logging.h>
#include <epidemiology/utils/random_number_generator.h>
#include <gtest/gtest.h>

#if HAVE_EPI_IO
#include <epidemiology_io/io.h>
#endif

int main(int argc, char** argv)
{
    epi::set_log_level(epi::LogLevel::warn);
    epi::log_thread_local_rng_seeds(epi::LogLevel::warn);

    ::testing::InitGoogleTest(&argc, argv);
    int retval = RUN_ALL_TESTS();
    return retval;
}
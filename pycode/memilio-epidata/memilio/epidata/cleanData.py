#############################################################################
# Copyright (C) 2020-2021 German Aerospace Center (DLR-SC)
#
# Authors: Kathrin Rack, Wadim Koslow
#
# Contact: Martin J. Kuehn <Martin.Kuehn@DLR.de>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#############################################################################
"""
@file cleanData.py

@brief Tool to delete data generated by some memilio.epidata packages

Data from following packages can be deleted:
- getCaseData.py
- getJHData.py
- getDIVIData.py
- getPopulationData
- getCommuterMobility
- getVaccinationData
- getTestingData
"""

import argparse
import os

from memilio.epidata import defaultDict as dd


def clean_data(
        all_data, cases, john_hopkins, population, divi, vaccination, commuter,
        testing, hospitalization, json, hdf5, txt, out_path):
    """! Function to delete data which is generated by memilio.epidata package

        Deletes all data or data from the different packages.

        @param all_data Defines if all possible data should be deleted. Default = False
        @param cases Defines if data generated by getCaseData.py should be deleted. Default = False
        @param john_hopkins Defines if data generated by getJHData.py should be deleted. Default = False
        @param population Defines if data generated by getPopulationData.py should be deleted. Default = False
        @param divi Defines if data generated by getDIVIData.py should be deleted. Default = False
        @param vaccination Defines if data generated by getVaccinationData.py should be deleted. Default = False
        @param commuter Defines if data generated by getCommuterData.py should be deleted. Default = False
        @param testing Defines if data generated by getTestingData.py should be deleted. Default = False
        @param hospitalization Defines if data generated by getHospitalizationData.py should be deleted. Default = False
        @param json Defines if files with ending json should be deleted. Default = False
        @param hdf5 Defines if files with ending hdf5 should be deleted. Default = False
        @param txt Defines if files with ending txt should be deletd. Default = False
        @param out_path Defines path where data should be deleted.
        """
    file_types = []
    if json:
        file_types.append("json")
    if hdf5:
        file_types.append("h5")
    if txt:
        file_types.append("txt")

    # TODO: get list from other packages which data they write.
    # Otherwise when changing anything this tool has to be changes as well

    if all_data:

        # TODO: make general dictionary with all countries used
        directories = ['Germany/', 'Spain/', 'France/',
                       'Italy/', 'US/', 'SouthKorea/', 'China/']

        # delete files in directory
        for cdir in directories:
            directory = os.path.join(out_path, cdir)

            try:
                files = os.listdir(directory)
            except FileNotFoundError:
                continue

            for item in files:
                if item.endswith(".json") or item.endswith(".h5"):
                    print("Deleting file ", os.path.join(directory, item))
                    os.remove(os.path.join(directory, item))

            # delete directories if empty
            try:
                os.rmdir(directory)
            except OSError:
                continue
            print("Deleting directory ", directory)

        # delete further jh files
        files = []
        try:
            files = os.listdir(out_path)
        except FileNotFoundError:
            pass

        for item in files:
            if item.endswith(".json") or item.endswith(".h5"):
                print("Deleting file ", os.path.join(out_path, item))
                os.remove(os.path.join(out_path, item))

    else:
        for ending in file_types:
            # john hopkins data has to be removed from different directories
            if john_hopkins:
                # TODO: make general dictionary with all countries used
                directories = ['Germany/', 'Spain/', 'France/',
                            'Italy/', 'US/', 'SouthKorea/', 'China/']

                # delete files in directory
                for cdir in directories:
                    directory = os.path.join(out_path, cdir)

                    try:
                        files = os.listdir(directory)
                    except FileNotFoundError:
                        continue

                    for item in files:
                        if item.endswith(ending) and "_jh" in item:
                            print("Deleting file ", os.path.join(directory, item))
                            os.remove(os.path.join(directory, item))

                    # delete directories
                    try:
                        os.rmdir(directory)
                    except OSError:
                        continue

                    print("Deleting directory ", directory)

                # delete further jh files
                files = []
                try:
                    files = os.listdir(out_path)
                except FileNotFoundError:
                    pass

                for item in files:
                    if item.endswith(ending):
                        if "_jh" in item or "JohnHopkins" in item:
                            print("Deleting file ", os.path.join(out_path, item))
                            os.remove(os.path.join(out_path, item))

            # other data is stored in the same folder
        
            directory = os.path.join(out_path, 'Germany/')
            files = []
            try:
                files = os.listdir(directory)
            except FileNotFoundError:
                pass

            filenames = []

            cases_filenames = ["cases", "Case"]
            divi_filenames = ["divi", "DIVI"]
            vaccination_filenames = ["vacc", "Vacc"]
            population_filenames = ["Popul", "popul", "migration.", "reg_key", "zensus"]
            commuter_filenames = ["bfa", 'commuter']            
            testing_filenames = ["testpos"]
            hospitalization_filenames = ["hospit", "Hospit"]

            if cases:
                filenames += cases_filenames
            if divi:
                filenames += divi_filenames
            if vaccination:
                filenames += vaccination_filenames
            if population:
                filenames += population_filenames
            if commuter:
                filenames += commuter_filenames
            if testing:
                filenames += testing_filenames
            if hospitalization:
                filenames += hospitalization_filenames

            for item in files:
                if item.endswith(ending):

                    for file in filenames:
                        if file in item:
                            print(
                                "Deleting file ", os.path.join(
                                    directory, item))
                            os.remove(os.path.join(directory, item))

                # delete directory if empty
                try:
                    os.rmdir(directory)
                    print("Deleting directory ", directory)
                except OSError:
                    pass

            if filenames == []:
                print("Please specify what should be deleted. See --help for details.")


def cli():
    """! Command line interface specific for clean data

    Via this interface it can defined which data should be deleted and where.
    Possibilities are

    - delete all
    - delete just cases, jh, population, divi, vaccination, commuter or testing
    - choose file format: json or hdf5
    - define path to files
    """

    out_path_default = dd.defaultDict['out_folder']

    parser = argparse.ArgumentParser()

    parser.add_argument(
        '-a', '--all-data',
        help='Deletes all data and folders which could be possibly written by the epidata package.',
        action='store_true')
    parser.add_argument('-ca', '--cases', help='Deletes just case data.',
                        action='store_true')
    parser.add_argument(
        '-j', '--john-hopkins',
        help='Deletes just data from John Hopkins university.',
        action='store_true')
    parser.add_argument('-p', '--population',
                        help='Deletes just population data.',
                        action='store_true')
    parser.add_argument('-d', '--divi', help='Deletes just DIVI data.',
                        action='store_true')
    parser.add_argument('-v', '--vaccination',
                        help='Deletes just vaccination data.',
                        action='store_true')
    parser.add_argument('-co', '--commuter',
                        help='Deletes just commuter data.',
                        action='store_true')
    parser.add_argument('-t', '--testing', help='Deletes just testing data.',
                        action='store_true')
    parser.add_argument('-ho', '--hospitalization',
                        help='Deletes just hospitalizaiton data.',
                        action='store_true')
                        
    parser.add_argument('-h5', '--hdf5', help='Deletes just hdf5 files.',
                        action='store_true')
    parser.add_argument('-js', '--json', help='Deletes json files.', 
                        action='store_true')
    parser.add_argument('-tx', '--txt', help='Deletes txt files.',
                        action='store_true')
    parser.add_argument(
        '-o', '--out_path', type=str, default=out_path_default,
        help='Defines folder for output.')

    args = parser.parse_args()

    return_args = [args.all_data, args.cases, args.john_hopkins, args.population,
                   args.divi, args.vaccination, args.commuter, args.testing, 
                   args.hospitalization, args.json, args.hdf5, args.txt, args.out_path]


    return return_args


def main():
    """! Main program entry."""

    [all_data, cases, john_hopkins, population, divi,
        vaccination, commuter, testing, hospitalization, json, hdf5, txt, out_path] = cli()

    clean_data(all_data, cases, john_hopkins, population, divi,
               vaccination, commuter, testing, hospitalization, json, hdf5, txt, out_path)


if __name__ == "__main__":
    main()

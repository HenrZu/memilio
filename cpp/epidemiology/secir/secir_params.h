#ifndef SECIR_PARAMS_H
#define SECIR_PARAMS_H

#include "epidemiology/utils/eigen.h"
#include "epidemiology/utils/uncertain_value.h"
#include "epidemiology/math/adapt_rk.h"
#include "epidemiology/secir/age_group.h"
#include "epidemiology/secir/uncertain_matrix.h"
#include "epidemiology/utils/parameter_set.h"
#include "epidemiology/utils/custom_index_array.h"

#include <vector>

namespace epi
{

/*******************************************
 * Define Parameters of the SECIHURD model *
 *******************************************/

/**
 * @brief the start day in the SECIR model
 * The start day defines in which season the simulation can be started
 * If the start day is 180 and simulation takes place from t0=0 to
 * tmax=100 the days 180 to 280 of the year are simulated
 */
struct StartDay
{
    using Type = double;
    static Type get_default(AgeGroup) {
        return 0.;
    }
    static std::string name() {
        return "StartDay";
    }
};


/**
 * @brief the seasonality in the SECIR model
 * the seasonality is given as (1+k*sin()) where the sine
 * curve is below one in summer and above one in winter
 */
struct Seasonality
{
    using Type = UncertainValue;
    static Type get_default(AgeGroup) {
        return Type(0.);
    }
    static std::string name() {
        return "Seasonality";
    }
};

/**
 * @brief the icu capacity in the SECIR model
 */
struct ICUCapacity
{
    using Type = UncertainValue;
    static Type get_default(AgeGroup) {
        return Type(std::numeric_limits<double>::max());
    }
    static std::string name() {
        return "ICUCapacity";
    }
};

/**
 * @brief the incubation time in the SECIR model
 * @param tinc incubation time in day unit
 */
struct IncubationTime
{
    using Type = CustomIndexArray<UncertainValue, AgeGroup>;
    static Type get_default(AgeGroup size) {
        return Type(size, 1.);
    }
    static std::string name() {
        return "IncubationTime";
    }
};

/**
 * @brief the infectious time for symptomatic cases that are infected but
 *        who do not need to be hsopitalized in the SECIR model in day unit
 */
struct InfectiousTimeMild
{
    using Type = CustomIndexArray<UncertainValue, AgeGroup>;
    static Type get_default(AgeGroup size) {
        return Type(size, 1.);
    }
    static std::string name() {
        return "InfectiousTimeMild";
    }
};

/**
 * @brief the serial interval in the SECIR model in day unit
 */
struct SerialInterval
{
    using Type = CustomIndexArray<UncertainValue, AgeGroup>;
    static Type get_default(AgeGroup size) {
        return Type(size, 1.);
    }
    static std::string name() {
        return "SerialInterval";
    }
};

/**
 * @brief the time people are 'simply' hospitalized before returning home in the SECIR model
 *        in day unit
 */
struct HospitalizedToHomeTime
{
    using Type = CustomIndexArray<UncertainValue, AgeGroup>;
    static Type get_default(AgeGroup size) {
        return Type(size, 1.);
    }
    static std::string name() {
        return "HospitalizedToHomeTime";
    }
};

/**
 * @brief the time people are infectious at home before 'simply' hospitalized in the SECIR model
 *        in day unit
 */
struct HomeToHospitalizedTime
{
    using Type = CustomIndexArray<UncertainValue, AgeGroup>;
    static Type get_default(AgeGroup size) {
        return Type(size, 1.);
    }
    static std::string name() {
        return "HomeToHospitalizedTime";
    }
};

/**
 * @brief the time people are 'simply' hospitalized before being treated by ICU in the SECIR model
 *        in day unit
 */
struct HospitalizedToICUTime
{
    using Type = CustomIndexArray<UncertainValue, AgeGroup>;
    static Type get_default(AgeGroup size) {
        return Type(size, 1.);
    }
    static std::string name() {
        return "HospitalizedToICUTime";
    }
};

/**
 * @brief the time people are treated by ICU before returning home in the SECIR model
 *        in day unit
 */
struct ICUToHomeTime
{
    using Type = CustomIndexArray<UncertainValue, AgeGroup>;
    static Type get_default(AgeGroup size) {
        return Type(size, 1.);
    }
    static std::string name() {
        return "ICUToHomeTime";
    }
};

/**
 * @brief the infectious time for asymptomatic cases in the SECIR model
 *        in day unit
 */
struct InfectiousTimeAsymptomatic
{
    using Type = CustomIndexArray<UncertainValue, AgeGroup>;
    static Type get_default(AgeGroup size) {
        return Type(size, 1.);
    }
    static std::string name() {
        return "InfectiousTimeAsymptomatic";
    }
};

/**
 * @brief the time people are treated by ICU before dying in the SECIR model
 *        in day unit
 */
struct ICUToDeathTime
{
    using Type = CustomIndexArray<UncertainValue, AgeGroup>;
    static Type get_default(AgeGroup size) {
        return Type(size, 1.);
    }
    static std::string name() {
        return "ICUToDeathTime";
    }
};

/**
* @brief probability of getting infected from a contact
*/
struct InfectionProbabilityFromContact
{
    using Type = CustomIndexArray<UncertainValue, AgeGroup>;
    static Type get_default(AgeGroup size) {
        return Type(size, 1.);
    }
    static std::string name() {
        return "InfectionProbabilityFromContact";
    }
};

/**
* @brief the relative carrier infectability
*/
struct RelativeCarrierInfectability
{
    using Type = CustomIndexArray<UncertainValue, AgeGroup>;
    static Type get_default(AgeGroup size) {
        return Type(size, 1.);
    }
    static std::string name() {
        return "RelativeCarrierInfectability";
    }
};

/**
* @brief the percentage of asymptomatic cases in the SECIR model
*/
struct AsymptoticCasesPerInfectious
{
    using Type = CustomIndexArray<UncertainValue, AgeGroup>;
    static Type get_default(AgeGroup size) {
        return Type(size, 0.);
    }
    static std::string name() {
        return "AsymptoticCasesPerInfectious";
    }
};

/**
* @brief the risk of infection from symptomatic cases in the SECIR model
*/
struct RiskOfInfectionFromSympomatic
{
    using Type = CustomIndexArray<UncertainValue, AgeGroup>;
    static Type get_default(AgeGroup size) {
        return Type(size, 0.);
    }
    static std::string name() {
        return "RiskOfInfectionFromSympomatic";
    }
};

/**
* @brief risk of infection from symptomatic cases increases as test and trace capacity is exceeded.
*/
struct MaxRiskOfInfectionFromSympomatic
{
    using Type = CustomIndexArray<UncertainValue, AgeGroup>;
    static Type get_default(AgeGroup size) {
        return Type(size, 0.);
    }
    static std::string name() {
        return "MaxRiskOfInfectionFromSympomatic";
    }
};

/**
* @brief the percentage of hospitalized patients per infected patients in the SECIR model
*/
struct HospitalizedCasesPerInfectious
{
    using Type = CustomIndexArray<UncertainValue, AgeGroup>;
    static Type get_default(AgeGroup size) {
        return Type(size, 0.);
    }
    static std::string name() {
        return "HospitalizedCasesPerInfectious";
    }
};

/**
* @brief the percentage of ICU patients per hospitalized patients in the SECIR model
*/
struct ICUCasesPerHospitalized
{
    using Type = CustomIndexArray<UncertainValue, AgeGroup>;
    static Type get_default(AgeGroup size) {
        return Type(size, 0.);
    }
    static std::string name() {
        return "ICUCasesPerHospitalized";
    }
};

/**
* @brief the percentage of dead patients per ICU patients in the SECIR model
*/
struct DeathsPerHospitalized
{
    using Type = CustomIndexArray<UncertainValue, AgeGroup>;
    static Type get_default(AgeGroup size) {
        return Type(size, 0.);
    }
    static std::string name() {
        return "DeathsPerHospitalized";
    }
};

/**
 * @brief the contact patterns within the society are modelled using an UncertainContactMatrix
 */
struct ContactPatterns
{
    using Type = UncertainContactMatrix;
    static Type get_default(AgeGroup size) {
        return Type(1, static_cast<Eigen::Index>((size_t)size));
    }
    static std::string name() {
        return "ContactPatterns";
    }
};

/**
 * @brief capacity to test and trace contacts of infected for quarantine per day.
 */
struct TestAndTraceCapacity
{
    using Type = UncertainValue;
    static Type get_default(AgeGroup) {
        return Type(std::numeric_limits<double>::max());
    }
    static std::string name() {
        return "TestAndTraceCapacity";
    }
};


using SecirParamsBase = ParameterSet<StartDay
                                    ,Seasonality
                                    ,ICUCapacity
                                    ,TestAndTraceCapacity
                                    ,ContactPatterns
                                    ,IncubationTime
                                    ,InfectiousTimeMild
                                    ,InfectiousTimeAsymptomatic
                                    ,SerialInterval
                                    ,HospitalizedToHomeTime
                                    ,HomeToHospitalizedTime
                                    ,HospitalizedToICUTime
                                    ,ICUToHomeTime
                                    ,ICUToDeathTime
                                    ,InfectionProbabilityFromContact
                                    ,RelativeCarrierInfectability
                                    ,AsymptoticCasesPerInfectious
                                    ,RiskOfInfectionFromSympomatic
                                    ,MaxRiskOfInfectionFromSympomatic
                                    ,HospitalizedCasesPerInfectious
                                    ,ICUCasesPerHospitalized
                                    ,DeathsPerHospitalized
                                    >;

/**
 * @brief Parameters of an age-resolved SECIR/SECIHURD model.
 */
class SecirParams : public SecirParamsBase
{
public:

    SecirParams(AgeGroup num_agegroups)
        : SecirParamsBase(num_agegroups)
        , m_num_groups{num_agegroups}
    {
    }

    AgeGroup get_num_groups() const
    {
        return m_num_groups;
    }



    /**
     * @brief checks whether all Parameters satisfy their corresponding constraints and applies them, if they do not
     */
    void apply_constraints()
    {
        if (this->get<Seasonality>() < 0.0 || this->get<Seasonality>() > 0.5) {
            log_warning("Constraint check: Parameter Seasonality changed from {:0.4f} to {:d}", this->get<Seasonality>(), 0);
            this->set<Seasonality>(0);
        }

        if (this->get<ICUCapacity>() < 0.0) {
            log_warning("Constraint check: Parameter ICUCapacity changed from {:0.4f} to {:d}", this->get<ICUCapacity>(), 0);
            this->set<ICUCapacity>(0);
        }

        for(auto i = AgeGroup(0); i < AgeGroup(m_num_groups); ++i) {

            if (this->get<IncubationTime>()[i] < 2.0) {
                log_warning("Constraint check: Parameter IncubationTime changed from {:.4f} to {:.4f}", this->get<IncubationTime>()[i], 2.0);
                this->get<IncubationTime>()[i] = 2.0;
            }

            if (2 * this->get<SerialInterval>()[i] < this->get<IncubationTime>()[i] + 1.0) {
                log_warning("Constraint check: Parameter SerialInterval changed from {:.4f} to {:.4f}", this->get<SerialInterval>()[i],
                            0.5 * this->get<IncubationTime>()[i] + 0.5);
                this->get<SerialInterval>()[i] = 0.5 * this->get<IncubationTime>()[i] + 0.5;
            }
            else if (this->get<SerialInterval>()[i] > this->get<IncubationTime>()[i] - 0.5) {
                log_warning("Constraint check: Parameter SerialInterval changed from {:.4f} to {:.4f}", this->get<SerialInterval>()[i], this->get<IncubationTime>()[i] - 0.5);
                this->get<SerialInterval>()[i] = this->get<IncubationTime>()[i] - 0.5;
            }

            if (this->get<InfectiousTimeMild>()[i] < 1.0) {
                log_warning("Constraint check: Parameter InfectiousTimeMild changed from {:.4f} to {:.4f}", this->get<InfectiousTimeMild>()[i], 1.0);
                this->get<InfectiousTimeMild>()[i] = 1.0;
            }

            if (this->get<HospitalizedToHomeTime>()[i] < 1.0) {
                log_warning("Constraint check: Parameter HospitalizedToHomeTime changed from {:.4f} to {:.4f}", this->get<HospitalizedToHomeTime>()[i], 1.0);
                this->get<HospitalizedToHomeTime>()[i] = 1.0;
            }

            if (this->get<HomeToHospitalizedTime>()[i] < 1.0) {
                log_warning("Constraint check: Parameter HomeToHospitalizedTime changed from {:.4f} to {:.4f}", this->get<HomeToHospitalizedTime>()[i], 1.0);
                this->get<HomeToHospitalizedTime>()[i] = 1.0;
            }

            if (this->get<HospitalizedToICUTime>()[i] < 1.0) {
                log_warning("Constraint check: Parameter HospitalizedToICUTime changed from {:.4f} to {:.4f}", this->get<HospitalizedToICUTime>()[i], 1.0);
                this->get<HospitalizedToICUTime>()[i] = 1.0;
            }

            if (this->get<ICUToHomeTime>()[i] < 1.0) {
                log_warning("Constraint check: Parameter ICUToHomeTime changed from {:.4f} to {:.4f}", this->get<ICUToHomeTime>()[i], 1.0);
                this->get<ICUToHomeTime>()[i] = 1.0;
            }

            if (this->get<ICUToDeathTime>()[i] < 1.0) {
                log_warning("Constraint check: Parameter ICUToDeathTime changed from {:.4f} to {:.4f}", this->get<ICUToDeathTime>()[i], 1.0);
                this->get<ICUToDeathTime>()[i] = 1.0;
            }

            if (this->get<InfectiousTimeAsymptomatic>()[i] != 1.0 / (0.5 / (this->get<InfectiousTimeAsymptomatic>()[i] - this->get<SerialInterval>()[i])) + 0.5 * this->get<InfectiousTimeMild>()[i]) {
                log_info("Constraint check: Parameter InfectiousTimeAsymptomatic set as fully dependent on tinc, tserint and tinfmild. See HZI "
                         "paper.");
                this->get<InfectiousTimeAsymptomatic>()[i] = 1.0 / (0.5 / (this->get<IncubationTime>()[i] - this->get<SerialInterval>()[i])) + 0.5 * this->get<InfectiousTimeMild>()[i];
            }

            if (this->get<InfectionProbabilityFromContact>()[i] < 0.0) {
                log_warning("Constraint check: Parameter InfectionProbabilityFromContact changed from {:0.4f} to {:d} ", this->get<InfectionProbabilityFromContact>()[i], 0);
                this->get<InfectionProbabilityFromContact>()[i] = 0;
            }

            if (this->get<RelativeCarrierInfectability>()[i] < 0.0) {
                log_warning("Constraint check: Parameter RelativeCarrierInfectability changed from {:0.4f} to {:d} ", this->get<RelativeCarrierInfectability>()[i], 0);
                this->get<RelativeCarrierInfectability>()[i] = 0;
            }

            if (this->get<AsymptoticCasesPerInfectious>()[i] < 0.0 || this->get<AsymptoticCasesPerInfectious>()[i] > 1.0) {
                log_warning("Constraint check: Parameter AsymptoticCasesPerInfectious changed from {:0.4f} to {:d} ", this->get<AsymptoticCasesPerInfectious>()[i], 0);
                this->get<AsymptoticCasesPerInfectious>()[i] = 0;
            }

            if (this->get<RiskOfInfectionFromSympomatic>()[i] < 0.0 || this->get<RiskOfInfectionFromSympomatic>()[i] > 1.0) {
                log_warning("Constraint check: Parameter RiskOfInfectionFromSympomatic changed from {:0.4f} to {:d}", this->get<RiskOfInfectionFromSympomatic>()[i], 0);
                this->get<RiskOfInfectionFromSympomatic>()[i] = 0;
            }

            if (this->get<HospitalizedCasesPerInfectious>()[i] < 0.0 || this->get<HospitalizedCasesPerInfectious>()[i] > 1.0) {
                log_warning("Constraint check: Parameter HospitalizedCasesPerInfectious changed from {:0.4f} to {:d}", this->get<HospitalizedCasesPerInfectious>()[i], 0);
                this->get<HospitalizedCasesPerInfectious>()[i] = 0;
            }

            if (this->get<ICUCasesPerHospitalized>()[i] < 0.0 || this->get<ICUCasesPerHospitalized>()[i] > 1.0) {
                log_warning("Constraint check: Parameter ICUCasesPerHospitalized changed from {:0.4f} to {:d}", this->get<ICUCasesPerHospitalized>()[i], 0);
                this->get<ICUCasesPerHospitalized>()[i] = 0;
            }

            if (this->get<DeathsPerHospitalized>()[i] < 0.0 || this->get<DeathsPerHospitalized>()[i] > 1.0) {
                log_warning("Constraint check: Parameter DeathsPerHospitalized changed from {:0.4f} to {:d}", this->get<DeathsPerHospitalized>()[i], 0);
                this->get<DeathsPerHospitalized>()[i] = 0;
            }
        }
    }

    /**
     * @brief checks whether all Parameters satisfy their corresponding constraints and throws errors, if they do not
     */
    void check_constraints() const
    {
        if (this->get<Seasonality>() < 0.0 || this->get<Seasonality>() > 0.5) {
            log_warning("Constraint check: Parameter m_seasonality smaller {:d} or larger {:d}", 0, 0.5);
        }

        if (this->get<ICUCapacity>() < 0.0) {
            log_warning("Constraint check: Parameter m_icu_capacity smaller {:d}", 0);
        }

        for(auto i = AgeGroup(0); i < AgeGroup(m_num_groups); ++i) {

            if (this->get<IncubationTime>()[i] < 2.0) {
                log_error("Constraint check: Parameter IncubationTime {:.4f} smaller {:.4f}", this->get<IncubationTime>()[i], 2.0);
            }

            if (2 * this->get<SerialInterval>()[i] < this->get<IncubationTime>()[i] + 1.0) {
                log_error("Constraint check: Parameter SerialInterval {:.4f} smaller {:.4f}", this->get<SerialInterval>()[i], 0.5 * this->get<IncubationTime>()[i] + 0.5);
            }
            else if (this->get<SerialInterval>()[i] > this->get<IncubationTime>()[i] - 0.5) {
                log_error("Constraint check: Parameter SerialInterval {:.4f} smaller {:.4f}", this->get<SerialInterval>()[i], this->get<IncubationTime>()[i] - 0.5);
            }

            if (this->get<InfectiousTimeMild>()[i] < 1.0) {
                log_error("Constraint check: Parameter InfectiousTimeMild {:.4f} smaller {:.4f}", this->get<InfectiousTimeMild>()[i], 1.0);
            }

            if (this->get<HospitalizedToHomeTime>()[i] < 1.0) {
                log_error("Constraint check: Parameter HospitalizedToHomeTime {:.4f} smaller {:.4f}", this->get<HospitalizedToHomeTime>()[i], 1.0);
            }

            if (this->get<HomeToHospitalizedTime>()[i] < 1.0) {
                log_error("Constraint check: Parameter HomeToHospitalizedTime {:.4f} smaller {:.4f}", this->get<HomeToHospitalizedTime>()[i], 1.0);
            }

            if (this->get<HospitalizedToICUTime>()[i] < 1.0) {
                log_error("Constraint check: Parameter HospitalizedToICUTime {:.4f} smaller {:.4f}", this->get<HospitalizedToICUTime>()[i], 1.0);
            }

            if (this->get<ICUToHomeTime>()[i] < 1.0) {
                log_error("Constraint check: Parameter ICUToHomeTime {:.4f} smaller {:.4f}", this->get<ICUToHomeTime>()[i], 1.0);
            }

            if (this->get<InfectiousTimeAsymptomatic>()[i] != 1.0 / (0.5 / (this->get<InfectiousTimeAsymptomatic>()[i] - this->get<SerialInterval>()[i])) + 0.5 * this->get<InfectiousTimeMild>()[i]) {
                log_error("Constraint check: Parameter InfectiousTimeAsymptomatic not set as fully dependent on tinc, tserint and tinfmild. See "
                              "HZI paper.");
            }


            if (this->get<ICUToDeathTime>()[i] < 1.0) {
                log_error("Constraint check: Parameter ICUToDeathTime {:.4f} smaller {:.4f}", this->get<ICUToDeathTime>()[i], 1.0);
            }

            if (this->get<InfectionProbabilityFromContact>()[i] < 0.0) {
                log_warning("Constraint check: Parameter InfectionProbabilityFromContact smaller {:d}", 0);
            }

            if (this->get<RelativeCarrierInfectability>()[i] < 0.0) {
                log_warning("Constraint check: Parameter RelativeCarrierInfectability smaller {:d}", 0);
            }

            if (this->get<AsymptoticCasesPerInfectious>()[i] < 0.0 || this->get<AsymptoticCasesPerInfectious>()[i] > 1.0) {
                log_warning("Constraint check: Parameter AsymptoticCasesPerInfectious smaller {:d} or larger {:d}", 0, 1);
            }

            if (this->get<RiskOfInfectionFromSympomatic>()[i] < 0.0 || this->get<RiskOfInfectionFromSympomatic>()[i] > 1.0) {
                log_warning("Constraint check: Parameter RiskOfInfectionFromSympomatic smaller {:d} or larger {:d}", 0, 1);
            }

            if (this->get<HospitalizedCasesPerInfectious>()[i] < 0.0 || this->get<HospitalizedCasesPerInfectious>()[i] > 1.0) {
                log_warning("Constraint check: Parameter HospitalizedCasesPerInfectious smaller {:d} or larger {:d}", 0, 1);
            }

            if (this->get<ICUCasesPerHospitalized>()[i] < 0.0 || this->get<ICUCasesPerHospitalized>()[i] > 1.0) {
                log_warning("Constraint check: Parameter ICUCasesPerHospitalized smaller {:d} or larger {:d}", 0, 1);
            }

            if (this->get<DeathsPerHospitalized>()[i] < 0.0 || this->get<DeathsPerHospitalized>()[i] > 1.0) {
                log_warning("Constraint check: Parameter DeathsPerHospitalized smaller {:d} or larger {:d}", 0, 1);
            }

        }

    }

private:

    AgeGroup m_num_groups;

};

/**
 * @brief WIP !! TO DO: returns the actual, approximated reproduction rate 
 */
//double get_reprod_rate(SecirParams const& params, double t, std::vector<double> const& yt);

} // namespace epi

#endif // SECIR_H

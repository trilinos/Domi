// @HEADER
// ****************************************************************************
//                Tempus: Copyright (2017) Sandia Corporation
//
// Distributed under BSD 3-clause license (See accompanying file Copyright.txt)
// ****************************************************************************
// @HEADER

#include "Teuchos_UnitTestHarness.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp"
#include "Teuchos_TimeMonitor.hpp"

#include "Thyra_VectorStdOps.hpp"

#include "Tempus_IntegratorBasic.hpp"

#include "Tempus_StepperFactory.hpp"
#include "Tempus_StepperSubcycling.hpp"

#include "../TestModels/SinCosModel.hpp"
#include "../TestModels/VanDerPolModel.hpp"
#include "../TestUtils/Tempus_ConvergenceTestUtils.hpp"

#include <fstream>
#include <vector>

namespace Tempus_Test {

using Teuchos::RCP;
using Teuchos::rcp;
using Teuchos::rcp_const_cast;
using Teuchos::ParameterList;
using Teuchos::sublist;
using Teuchos::getParametersFromXmlFile;

using Tempus::IntegratorBasic;
using Tempus::SolutionHistory;
using Tempus::SolutionState;


// Comment out any of the following tests to exclude from build/run.
//#define TEST_PARAMETERLIST
#define TEST_CONSTRUCTING_FROM_DEFAULTS
//#define TEST_SINCOS


#ifdef TEST_PARAMETERLIST
// ************************************************************
// ************************************************************
TEUCHOS_UNIT_TEST(Subcycling, ParameterList)
{
  // Read params from .xml file
  RCP<ParameterList> pList =
    getParametersFromXmlFile("Tempus_Subcycling_SinCos.xml");

  //std::ofstream ftmp("PL.txt");
  //pList->print(ftmp);
  //ftmp.close();

  // Setup the SinCosModel
  RCP<ParameterList> scm_pl = sublist(pList, "SinCosModel", true);
  auto model = rcp(new SinCosModel<double> (scm_pl));

  RCP<ParameterList> tempusPL  = sublist(pList, "Tempus", true);

  // Test constructor IntegratorBasic(tempusPL, model)
  {
    RCP<Tempus::IntegratorBasic<double> > integrator =
      Tempus::integratorBasic<double>(tempusPL, model);

    RCP<ParameterList> stepperPL = sublist(tempusPL, "Demo Stepper", true);
    RCP<const ParameterList> defaultPL =
      integrator->getStepper()->getValidParameters();

    bool pass = haveSameValues(*stepperPL, *defaultPL, true);
    if (!pass) {
      std::cout << std::endl;
      std::cout << "stepperPL -------------- \n" << *stepperPL << std::endl;
      std::cout << "defaultPL -------------- \n" << *defaultPL << std::endl;
    }
    TEST_ASSERT(pass)
  }

  // Test constructor IntegratorBasic(model, stepperType)
  {
    RCP<Tempus::IntegratorBasic<double> > integrator =
      Tempus::integratorBasic<double>(model, "Forward Euler");

    RCP<ParameterList> stepperPL = sublist(tempusPL, "Demo Stepper", true);
    RCP<const ParameterList> defaultPL =
      integrator->getStepper()->getValidParameters();

    bool pass = haveSameValues(*stepperPL, *defaultPL, true);
    if (!pass) {
      std::cout << std::endl;
      std::cout << "stepperPL -------------- \n" << *stepperPL << std::endl;
      std::cout << "defaultPL -------------- \n" << *defaultPL << std::endl;
    }
    TEST_ASSERT(pass)
  }
}
#endif // TEST_PARAMETERLIST


#ifdef TEST_CONSTRUCTING_FROM_DEFAULTS
// ************************************************************
// ************************************************************
TEUCHOS_UNIT_TEST(Subcycling, ConstructingFromDefaults)
{
  double dt = 0.4;

  // Setup the SinCosModel
  auto model = rcp(new SinCosModel<double>());

  // Setup Stepper for field solve ----------------------------
  auto stepper = rcp(new Tempus::StepperSubcycling<double>());
  auto sf = Teuchos::rcp(new Tempus::StepperFactory<double>());
  auto stepperFE = sf->createStepperForwardEuler(model, Teuchos::null);
  stepper->setSubcyclingStepper(stepperFE);

  stepper->setSubcyclingMinTimeStep      (0.1);
  stepper->setSubcyclingInitTimeStep     (0.1);
  stepper->setSubcyclingMaxTimeStep      (0.1);
  stepper->setSubcyclingStepType         ("Constant");
  stepper->setSubcyclingMaxFailures      (10);
  stepper->setSubcyclingMaxConsecFailures(5);
  stepper->setSubcyclingScreenOutputIndexInterval(1);

  stepper->initialize();

  // Setup TimeStepControl ------------------------------------
  auto timeStepControl = rcp(new Tempus::TimeStepControl<double>());
  timeStepControl->setStepType ("Constant");
  timeStepControl->setInitIndex(0);
  timeStepControl->setInitTime (0.0);
  timeStepControl->setFinalTime(1.0);
  timeStepControl->setInitTimeStep(dt);
  timeStepControl->initialize();

  // Setup initial condition SolutionState --------------------
  Thyra::ModelEvaluatorBase::InArgs<double> inArgsIC =
    stepper->getModel()->getNominalValues();
  auto icSolution = rcp_const_cast<Thyra::VectorBase<double> > (inArgsIC.get_x());
  auto icState = rcp(new Tempus::SolutionState<double>(icSolution));
  icState->setTime    (timeStepControl->getInitTime());
  icState->setIndex   (timeStepControl->getInitIndex());
  icState->setTimeStep(0.0);  // dt for ICs are indicated by zero.
  icState->setSolutionStatus(Tempus::Status::PASSED);  // ICs are passing.

  // Setup SolutionHistory ------------------------------------
  auto solutionHistory = rcp(new Tempus::SolutionHistory<double>());
  solutionHistory->setName("Forward States");
  solutionHistory->setStorageType(Tempus::STORAGE_TYPE_STATIC);
  solutionHistory->setStorageLimit(2);
  solutionHistory->addState(icState);

  // Setup Integrator -----------------------------------------
  RCP<Tempus::IntegratorBasic<double> > integrator =
    Tempus::integratorBasic<double>();
  integrator->setStepperWStepper(stepper);
  integrator->setTimeStepControl(timeStepControl);
  integrator->setSolutionHistory(solutionHistory);
  integrator->setScreenOutputIndexInterval(1);
  //integrator->setObserver(...);
  integrator->initialize();


  // Integrate to timeMax
  bool integratorStatus = integrator->advanceTime();
  TEST_ASSERT(integratorStatus)


  // Test if at 'Final Time'
  double time = integrator->getTime();
  double timeFinal = 1.0;
  TEST_FLOATING_EQUALITY(time, timeFinal, 1.0e-14);

  // Time-integrated solution and the exact solution
  RCP<Thyra::VectorBase<double> > x = integrator->getX();
  RCP<const Thyra::VectorBase<double> > x_exact =
    model->getExactSolution(time).get_x();

  // Calculate the error
  RCP<Thyra::VectorBase<double> > xdiff = x->clone_v();
  Thyra::V_StVpStV(xdiff.ptr(), 1.0, *x_exact, -1.0, *(x));

  // Check the order and intercept
  std::cout << "  Stepper = " << stepper->description() << std::endl;
  std::cout << "  =========================" << std::endl;
  std::cout << "  Exact solution   : " << get_ele(*(x_exact), 0) << "   "
                                       << get_ele(*(x_exact), 1) << std::endl;
  std::cout << "  Computed solution: " << get_ele(*(x      ), 0) << "   "
                                       << get_ele(*(x      ), 1) << std::endl;
  std::cout << "  Difference       : " << get_ele(*(xdiff  ), 0) << "   "
                                       << get_ele(*(xdiff  ), 1) << std::endl;
  std::cout << "  =========================" << std::endl;
  TEST_FLOATING_EQUALITY(get_ele(*(x), 0), 0.882508, 1.0e-4 );
  TEST_FLOATING_EQUALITY(get_ele(*(x), 1), 0.570790, 1.0e-4 );
}
#endif // TEST_CONSTRUCTING_FROM_DEFAULTS


#ifdef TEST_SINCOS
// ************************************************************
// ************************************************************
TEUCHOS_UNIT_TEST(Subcycling, SinCos)
{
  RCP<Tempus::IntegratorBasic<double> > integrator;
  std::vector<RCP<Thyra::VectorBase<double>>> solutions;
  std::vector<RCP<Thyra::VectorBase<double>>> solutionsDot;
  std::vector<double> StepSize;
  std::vector<double> xErrorNorm;
  std::vector<double> xDotErrorNorm;
  const int nTimeStepSizes = 7;
  double dt = 0.2;
  double time = 0.0;
  for (int n=0; n<nTimeStepSizes; n++) {

    // Read params from .xml file
    RCP<ParameterList> pList =
      getParametersFromXmlFile("Tempus_Subcycling_SinCos.xml");

    //std::ofstream ftmp("PL.txt");
    //pList->print(ftmp);
    //ftmp.close();

    // Setup the SinCosModel
    RCP<ParameterList> scm_pl = sublist(pList, "SinCosModel", true);
    //RCP<SinCosModel<double> > model = sineCosineModel(scm_pl);
    auto model = rcp(new SinCosModel<double> (scm_pl));

    dt /= 2;

    // Setup the Integrator and reset initial time step
    RCP<ParameterList> pl = sublist(pList, "Tempus", true);
    pl->sublist("Demo Integrator")
       .sublist("Time Step Control").set("Initial Time Step", dt);
    integrator = Tempus::integratorBasic<double>(pl, model);

    // Initial Conditions
    // During the Integrator construction, the initial SolutionState
    // is set by default to model->getNominalVales().get_x().  However,
    // the application can set it also by integrator->initializeSolutionHistory.
    RCP<Thyra::VectorBase<double> > x0 =
      model->getNominalValues().get_x()->clone_v();
    integrator->initializeSolutionHistory(0.0, x0);

    // Integrate to timeMax
    bool integratorStatus = integrator->advanceTime();
    TEST_ASSERT(integratorStatus)

    // Test PhysicsState
    RCP<Tempus::PhysicsState<double> > physicsState =
      integrator->getSolutionHistory()->getCurrentState()->getPhysicsState();
    TEST_EQUALITY(physicsState->getName(), "Tempus::PhysicsState");

    // Test if at 'Final Time'
    time = integrator->getTime();
    double timeFinal = pl->sublist("Demo Integrator")
      .sublist("Time Step Control").get<double>("Final Time");
    TEST_FLOATING_EQUALITY(time, timeFinal, 1.0e-14);

    // Time-integrated solution and the exact solution
    RCP<Thyra::VectorBase<double> > x = integrator->getX();
    RCP<const Thyra::VectorBase<double> > x_exact =
      model->getExactSolution(time).get_x();

    // Plot sample solution and exact solution
    if (n == 0) {
      RCP<const SolutionHistory<double> > solutionHistory =
        integrator->getSolutionHistory();
      writeSolution("Tempus_Subcycling_SinCos.dat", solutionHistory);

      auto solnHistExact = rcp(new Tempus::SolutionHistory<double>());
      for (int i=0; i<solutionHistory->getNumStates(); i++) {
        double time_i = (*solutionHistory)[i]->getTime();
        auto state = rcp(new Tempus::SolutionState<double>(
            model->getExactSolution(time_i).get_x(),
            model->getExactSolution(time_i).get_x_dot()));
        state->setTime((*solutionHistory)[i]->getTime());
        solnHistExact->addState(state);
      }
      writeSolution("Tempus_Subcycling_SinCos-Ref.dat", solnHistExact);
    }

    // Store off the final solution and step size
    StepSize.push_back(dt);
    auto solution = Thyra::createMember(model->get_x_space());
    Thyra::copy(*(integrator->getX()),solution.ptr());
    solutions.push_back(solution);
    auto solutionDot = Thyra::createMember(model->get_x_space());
    Thyra::copy(*(integrator->getXdot()),solutionDot.ptr());
    solutionsDot.push_back(solutionDot);
    if (n == nTimeStepSizes-1) {  // Add exact solution last in vector.
      StepSize.push_back(0.0);
      auto solutionExact = Thyra::createMember(model->get_x_space());
      Thyra::copy(*(model->getExactSolution(time).get_x()),solutionExact.ptr());
      solutions.push_back(solutionExact);
      auto solutionDotExact = Thyra::createMember(model->get_x_space());
      Thyra::copy(*(model->getExactSolution(time).get_x_dot()),
                  solutionDotExact.ptr());
      solutionsDot.push_back(solutionDotExact);
    }
  }

  // Check the order and intercept
  double xSlope = 0.0;
  double xDotSlope = 0.0;
  RCP<Tempus::Stepper<double> > stepper = integrator->getStepper();
  double order = stepper->getOrder();
  writeOrderError("Tempus_Subcycling_SinCos-Error.dat",
                  stepper, StepSize,
                  solutions,    xErrorNorm,    xSlope,
                  solutionsDot, xDotErrorNorm, xDotSlope);

  TEST_FLOATING_EQUALITY( xSlope,               order, 0.01   );
  TEST_FLOATING_EQUALITY( xErrorNorm[0],     0.051123, 1.0e-4 );
  // xDot not yet available for Forward Euler.
  //TEST_FLOATING_EQUALITY( xDotSlope,            order, 0.01   );
  //TEST_FLOATING_EQUALITY( xDotErrorNorm[0], 0.0486418, 1.0e-4 );

  Teuchos::TimeMonitor::summarize();
}
#endif // TEST_SINCOS


} // namespace Tempus_Test

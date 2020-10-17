#include "lambda.hpp"

// The lambda term to be evaluated.
const LambdaTerm& MAIN = (
    SIGN, (IEXP, ITWON, ITWOP)
);

/**
 * A program that evaluates a Lambda Calculus term using beta-reduction.
 * The Lambda Calculus term to be evaluated is defined as MAIN.
 * The evaluation details can be displayed with VERBOSTY in lambda.hpp.
 * C++ syntax or Lambda Calculus syntax (defined in
 *   lambda.hpp) can be used to define lambda terms.
 */
int main() noexcept {
    // Logs the reduction summary of MAIN.
    std::stringstream logger;
    
    // Main is displayed.
    if (VERBOSITY != SUMMARY) {
        std::cout
            << std::endl
            << "MAIN := "
            << MAIN
            << std::endl
        ;
    }
    
    if (VERBOSITY >= SUMMARY) {
        logger
            << std::endl
            << "MAIN := "
            << MAIN
            << std::endl
        ;
    }
    
    // Main is reduced.
    std::unique_ptr<LambdaTerm> reduced(MAIN.reduce());
    
    while (MAIN.to_string() != reduced->to_string()) {
        if (VERBOSITY >= SUMMARY) {
            if (VERBOSITY >= VERBOSE) {
                // The reduced from is displayed.
                std::cout
                    << std::endl
                    << "= "
                    << *reduced
                    << std::endl
                ;
            }
        
            logger
                << std::endl
                << "= "
                << *reduced
                << std::endl
            ;
        }
        
        // Further reduction is attempted.
        std::unique_ptr<LambdaTerm> new_reduced(reduced->reduce());
        
        // If further reduction achieves nothing, the loop is terminated.
        if (reduced->to_string() == new_reduced->to_string()) {
            break;
        }
        
        // Else, the reduced form is updated.
        else {
            reduced = std::move(new_reduced);
        }
    }
    
    // The reduced from is displayed a final time.
    if (VERBOSITY != SUMMARY) {
        std::cout
            << std::endl
            << "= "
            << *reduced
            << std::endl
        ;
    }
    
    if (VERBOSITY >= SUMMARY) {
        if (VERBOSITY >= VERBOSE) {
            std::cout << "\n\n\nSummary:" << std::endl;
        }
        
        std::cout << logger.str();
    }
    
    return 0;
}
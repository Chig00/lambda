#include <iostream>
#include <memory>

// Stringable Class
//{
/**
 * An abstract base class for objects with the to_string() method.
 */
class Stringable {
    public:
        /**
         * Returns a string representation of the object.
         */
        virtual std::string to_string() const noexcept = 0;
};

/**
 * An overload for the stream insertion operator for Stringable objects.
 */
std::ostream& operator<<(std::ostream& stream, const Stringable& stringable) noexcept {
    return stream << stringable.to_string();
}
//}

// Lambda Term Classes
//{
// Class Definitions
//{
class Variable;

/**
 * An abstract base class for lambda terms to inherit from.
 */
class LambdaTerm: public Stringable {
    public:
        /**
         * Attempts to reduce the lambda-term to produce a beta-normal form.
         */
        virtual std::unique_ptr<LambdaTerm> reduce() const noexcept = 0;
        
        /**
         * Attempts to apply itself to the given argument.
         */
        virtual std::unique_ptr<LambdaTerm> apply(const LambdaTerm&) const noexcept = 0;
        
        /**
         * Attempts to substitute the argument into the matching variables.
         */
        virtual std::unique_ptr<LambdaTerm> substitute(
            const Variable&,
            const LambdaTerm&
        ) const noexcept = 0;
};

/**
 * A lambda term that represents a parameter or value.
 */
class Variable: public LambdaTerm {
    public:
        /**
         * Constructs a new variable with its name.
         */
        Variable(const std::string& name) noexcept: name(name) {}
        
        /**
         * Constructs a new variable with its name (in C-string form).
         */
        Variable(const char* name) noexcept: name(name) {}
        
        /**
         * Returns true if the two variables have the same name.
         */
        bool operator==(const Variable& variable) const noexcept {
            return variable.name == name;
        }
        
        /**
         * Returns the variable's name.
         */
        std::string to_string() const noexcept {
            return name;
        }
        
        /**
         * Returns itself, as variables cannot be reduced.
         */
        std::unique_ptr<LambdaTerm> reduce() const noexcept {
            return std::make_unique<Variable>(*this);
        }
        
        /**
         * Returns itself applied to the given argument.
         */
        std::unique_ptr<LambdaTerm> apply(const LambdaTerm&) const noexcept;
        
        /**
         * Returns the term passed if the variable passed is equal to this one.
         * Returns this variable otherwise.
         */
        std::unique_ptr<LambdaTerm> substitute(
            const Variable&,
            const LambdaTerm&
        ) const noexcept;
        
    private:
        std::string name; // The variable's name.
};

/**
 * A lambda term that represents a function definition.
 */
class Abstraction: public LambdaTerm {
    public:
        /**
         * Constructs a new abstraction with its variable and definition.
         */
        template <typename Definition>
        Abstraction(
            const Variable& variable,
            const Definition& definition
        ) noexcept:
            variable(variable),
            definition(std::make_unique<Definition>(definition))
        {}
        
        /**
         * Creates a new abstraction as a copy of another.
         */
        Abstraction(const Abstraction& abstraction) noexcept;
        
        /**
         * Returns the function in string form.
         */
        std::string to_string() const noexcept {
            return
                "(\\"
                + variable.to_string()
                + '.'
                + definition->to_string()
                + ')'
            ;
        }
        
        /**
         * Attempts to reduce the function definition to produce a beta-normal form.
         */
        std::unique_ptr<LambdaTerm> reduce() const noexcept {
            std::unique_ptr<Abstraction> reduced(std::make_unique<Abstraction>(*this));
            reduced->definition = definition->reduce();
            return reduced;
        }
        
        /**
         * Substitutes in the value of the argument into the definition.
         */
        std::unique_ptr<LambdaTerm> apply(const LambdaTerm& argument) const noexcept {
            return definition->substitute(variable, argument);
        }
        
        /**
         * Attempts to substitute the term into the definition, if the
         *   variable passed is not equal to the abstraction's variable.
         */
        virtual std::unique_ptr<LambdaTerm> substitute(
            const Variable& variable,
            const LambdaTerm& term
        ) const noexcept {
            // There is name collision and this function takes precedence.
            if (variable == this->variable) {
                return std::make_unique<Abstraction>(*this);
            }
            
            // The function definition is substituted into.
            else {
                std::unique_ptr<Abstraction> subbed(std::make_unique<Abstraction>(*this));
                subbed->definition = definition->substitute(variable, term);
                return subbed;
            }
        }
        
    private:
        Variable variable;                      // The variable to substitute in.
        std::unique_ptr<LambdaTerm> definition; // The function to be substituted into.
};

/**
 * A lambda term that represents a function being applied to an argument.
 */
class Application: public LambdaTerm {
    public:
        /**
         * Constructs a new application with its function and argument.
         */
        template<typename Function, typename Argument>
        Application(
            const Function& function,
            const Argument& argument
        ) noexcept:
            function(std::make_unique<Function>(function)),
            argument(std::make_unique<Argument>(argument))
        {}
        
        /**
         * Creates a new application as a copy of another.
         */
        Application(const Application& application) noexcept {
            // The function is a variable.
            if (dynamic_cast<Variable*>(&*application.function)) {
                function =
                    std::make_unique<Variable>(
                        static_cast<Variable&>(*application.function)
                    )
                ;
            }
            
            // The function is an application.
            else if (dynamic_cast<Abstraction*>(&*application.function)) {
                function =
                    std::make_unique<Abstraction>(
                        static_cast<Abstraction&>(*application.function)
                    )
                ;
            }
            
            // The function is an application.
            else {
                function =
                    std::make_unique<Application>(
                        static_cast<Application&>(*application.function)
                    )
                ;
            }
            
            // The argument is a variable.
            if (dynamic_cast<Variable*>(&*application.argument)) {
                argument =
                    std::make_unique<Variable>(
                        static_cast<Variable&>(*application.argument)
                    )
                ;
            }
            
            // The argument is an application.
            else if (dynamic_cast<Abstraction*>(&*application.argument)) {
                argument =
                    std::make_unique<Abstraction>(
                        static_cast<Abstraction&>(*application.argument)
                    )
                ;
            }
            
            // The argument is an application.
            else {
                argument =
                    std::make_unique<Application>(
                        static_cast<Application&>(*application.argument)
                    )
                ;
            }
        }
        
        /**
         * Returns the application in string form.
         */
        std::string to_string() const noexcept {
            return
                function->to_string()
                + ' '
                + argument->to_string()
            ;
        }
        
        /**
         * Attempts to apply the function to the argument to produce a beta-normal form.
         */
        std::unique_ptr<LambdaTerm> reduce() const noexcept {
            return function->apply(*argument);
        }
        
        /**
         * This application is reduced before applying to the argument.
         */
        std::unique_ptr<LambdaTerm> apply(const LambdaTerm& argument) const noexcept {
            return reduce()->apply(argument);
        }
        
        /**
         * Attempts to substitute the term into the function and argument.
         */
        virtual std::unique_ptr<LambdaTerm> substitute(
            const Variable& variable,
            const LambdaTerm& term
        ) const noexcept {
            std::unique_ptr<Application> subbed(std::make_unique<Application>(*this));
            subbed->function = function->substitute(variable, term);
            subbed->argument = argument->substitute(variable, term);
            return subbed;
        }
        
    private:
        std::unique_ptr<LambdaTerm> function; // The function being applied.
        std::unique_ptr<LambdaTerm> argument; // The argument being applied to.
};
//}

// External Class Methods
//{
/**
 * Returns itself applied to the given argument.
 */
std::unique_ptr<LambdaTerm> Variable::apply(const LambdaTerm& argument) const noexcept {
    // The argument was a variable.
    if (dynamic_cast<const Variable*>(&argument)) {
        return
            std::make_unique<Application>(
                *this,
                static_cast<const Variable&>(argument)
            )
        ;
    }
    
    // The argument was an abstraction.
    else if (dynamic_cast<const Abstraction*>(&argument)) {
        return
            std::make_unique<Application>(
                *this,
                static_cast<const Abstraction&>(argument)
            )
        ;
    }
    
    // The argument was an application.
    else {
        return
            std::make_unique<Application>(
                *this,
                static_cast<const Application&>(argument)
            )
        ;
    }
}

/**
 * Returns the term passed if the variable passed is equal to this one.
 * Returns this variable otherwise.
 */
std::unique_ptr<LambdaTerm> Variable::substitute(
    const Variable& variable,
    const LambdaTerm& term
) const noexcept {
    // The variable should be substituted.
    if (operator==(variable)) {
        // The term is a variable.
        if (dynamic_cast<const Variable*>(&term)) {
            return
                std::make_unique<Variable>(
                    static_cast<const Variable&>(term)
                )
            ;
        }
        
        // The term is an abstraction.
        else if (dynamic_cast<const Abstraction*>(&term)) {
            return
                std::make_unique<Abstraction>(
                    static_cast<const Abstraction&>(term)
                )
            ;
        }
        
        // The term is an application.
        else {
            return
                std::make_unique<Application>(
                    static_cast<const Application&>(term)
                )
            ;
        }
    }
    
    // The variable is returned unchanged.
    else {
        return std::make_unique<Variable>(*this);
    }
}

/**
 * Creates a new abstraction as a copy of another.
 */
Abstraction::Abstraction(const Abstraction& abstraction) noexcept:
    variable(abstraction.variable)
{
    // The definition is a variable.
    if (dynamic_cast<Variable*>(&*abstraction.definition)) {
        definition =
            std::make_unique<Variable>(
                static_cast<Variable&>(*abstraction.definition)
            )
        ;
    }
    
    // The definition is an abstraction.
    else if (dynamic_cast<Abstraction*>(&*abstraction.definition)) {
        definition =
            std::make_unique<Abstraction>(
                static_cast<Abstraction&>(*abstraction.definition)
            )
        ;
    }
    
    // The definition is an application.
    else if (dynamic_cast<Application*>(&*abstraction.definition)) {
        definition =
            std::make_unique<Application>(
                static_cast<Application&>(*abstraction.definition)
            )
        ;
    }
}
//}

// Alternative Syntax
//{
/**
 * Alternative syntax for abstraction.
 */
template <typename Definition>
Abstraction operator>>(const Variable& variable, const Definition& definition) noexcept {
    return
        Abstraction(
            variable,
            definition
        )
    ;
}

/**
 * Alternative syntax for application.
 */
template <typename Function, typename Argument>
Application operator,(const Function& function, const Argument& argument) noexcept {
    return
        Application(
            function,
            argument
        )
    ;
}
//}
//}

// Lambda Term Definitions
//{
#include "lambda.hpp"
//}

// Main Function
//{
/**
 * Evaluates a lambda term.
 */
int main() noexcept {
    // Main is displayed.
    std::cout
        << std::endl
        << MAIN
        << std::endl
    ;
    
    // Main is reduced.
    std::unique_ptr<LambdaTerm> reduced = MAIN.reduce();
    
    while (true) {
        // The reduced from is displayed.
        std::cout
            << std::endl
            << "= "
            << *reduced
            << std::endl
        ;
        
        // Further reduction is attempted.
        std::unique_ptr<LambdaTerm> new_reduced = reduced->reduce();
        
        // If further reduction achieves nothing, the loop is terminated.
        if (reduced->to_string() == new_reduced->to_string()) {
            break;
        }
        
        // Else, the reduced form is updated.
        else {
            reduced = std::move(new_reduced);
        }
    }
    
    return 0;
}
//}
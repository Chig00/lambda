#include <iostream>
#include <sstream>
#include <memory>
#include <cstdlib>

// System
//{
// Verbosity
//{
/**
 * Defines the level of verbosity to be used in the program.
 */
enum Verbosity {
    BASIC,
    SUMMARY,
    VERBOSE
};

// The verbosity level to be used.
constexpr Verbosity VERBOSITY = BASIC;
//}

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
         * Default constructor.
         * Defualt variables are invalid.
         */
        Variable() noexcept: name("") {}
    
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
            if (VERBOSITY >= VERBOSE) {
                std::cout
                    << std::endl
                    << "Reduction of "
                    << *this
                    << std::endl
                ;
            }
            
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
        Abstraction(const Abstraction& abstraction) noexcept {
            operator=(abstraction);
        }
        
        /**
         * Copies the given abstraction.
         */
        Abstraction& operator=(const Abstraction& abstraction) noexcept;
        
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
            if (VERBOSITY >= VERBOSE) {
                std::cout
                    << std::endl
                    << "Reduction of "
                    << *this
                    << std::endl
                ;
            }
            
            std::unique_ptr<Abstraction> reduced(std::make_unique<Abstraction>(*this));
            reduced->definition = definition->reduce();
            return reduced;
        }
        
        /**
         * Substitutes in the value of the argument into the definition.
         */
        std::unique_ptr<LambdaTerm> apply(const LambdaTerm& argument) const noexcept;
        
        /**
         * Attempts to substitute the term into the definition, if the
         *   variable passed is not equal to the abstraction's variable.
         */
        std::unique_ptr<LambdaTerm> substitute(
            const Variable& variable,
            const LambdaTerm& term
        ) const noexcept {
            if (VERBOSITY >= VERBOSE) {
                std::cout
                    << std::endl
                    << "Substitution of "
                    << term
                    << " for "
                    << variable
                    << " in "
                    << *this
                    << std::endl
                ;
            }
            
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
            operator=(application);
        }
        
        /**
         * Copies the given application.
         */
        Application& operator=(const Application& application) noexcept {
            // Self-assignment check.
            if (this != &application) {
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
            
            // Self-reference returned.
            return *this;
        }
        
        /**
         * Returns the application in string form.
         */
        std::string to_string() const noexcept {
            return
                '['
                + function->to_string()
                + ' '
                + argument->to_string()
                + ']'
            ;
        }
        
        /**
         * Attempts to apply the function to the argument to produce a beta-normal form.
         */
        std::unique_ptr<LambdaTerm> reduce() const noexcept {
            if (VERBOSITY >= VERBOSE) {
                std::cout
                    << std::endl
                    << "Reduction of "
                    << *this
                    << std::endl
                ;
            }
            
            // Application of variables to arguments should not be attempted.
            if (dynamic_cast<Variable*>(&*function)) {
                std::unique_ptr<Application> reduced(std::make_unique<Application>(*this));
                reduced->argument = argument->reduce();
                return reduced;
            }
            
            else {
                return function->apply(*argument);
            }
        }
        
        /**
         * This application is reduced before applying to the argument.
         */
        std::unique_ptr<LambdaTerm> apply(const LambdaTerm& argument) const noexcept {
            if (VERBOSITY >= VERBOSE) {
                std::cout
                    << std::endl
                    << "Application of "
                    << *this
                    << " to "
                    << argument
                    << std::endl
                ;
            }
            
            std::unique_ptr<LambdaTerm> reduced(reduce());
            
            // The reduction yielded no change.
            if (reduced->to_string() == to_string()) {
                // The argument is a variable.
                if (dynamic_cast<const Variable*>(&argument)) {
                    return
                        std::make_unique<Application>(
                            *this,
                            static_cast<const Variable&>(argument)
                        )
                    ;
                }
                
                // The argument is an abstraction.
                else if (dynamic_cast<const Abstraction*>(&argument)) {
                    return
                        std::make_unique<Application>(
                            *this,
                            static_cast<const Abstraction&>(argument)
                        )
                    ;
                }
                
                // The argument is an application.
                else {
                    return
                        std::make_unique<Application>(
                            *this,
                            static_cast<const Application&>(argument)
                        )
                    ;
                }
            }
            
            else {
                return reduced->apply(argument);
            }
        }
        
        /**
         * Attempts to substitute the term into the function and argument.
         */
        std::unique_ptr<LambdaTerm> substitute(
            const Variable& variable,
            const LambdaTerm& term
        ) const noexcept {
            if (VERBOSITY >= VERBOSE) {
                std::cout
                    << std::endl
                    << "Substitution of "
                    << term
                    << " for "
                    << variable
                    << " in "
                    << *this
                    << std::endl
                ;
            }
            
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
 * If the argument is an application, reduction is attempted.
 */
std::unique_ptr<LambdaTerm> Variable::apply(const LambdaTerm& argument) const noexcept {
    if (VERBOSITY >= VERBOSE) {
        std::cout
            << std::endl
            << "Application of "
            << *this
            << " to "
            << argument
            << std::endl
        ;
    }

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
                static_cast<const Application&>(*argument.reduce())
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
    if (VERBOSITY >= VERBOSE) {
        std::cout
            << std::endl
            << "Substitution of "
            << term
            << " for "
            << variable
            << " in "
            << *this
            << std::endl
        ;
    }

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
 * Copies the given abstraction.
 */
Abstraction& Abstraction::operator=(const Abstraction& abstraction) noexcept {
    // Self-assignment check.
    if (this != &abstraction) {
        // The variable is copied.
        variable = abstraction.variable;
        
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
    
    // Self-reference returned.
    return *this;
}

/**
 * Substitutes in the value of the argument into the definition.
 */
std::unique_ptr<LambdaTerm> Abstraction::apply(const LambdaTerm& argument) const noexcept {
    if (VERBOSITY >= VERBOSE) {
        std::cout
            << std::endl
            << "Application of "
            << *this
            << " to "
            << argument
            << std::endl
        ;
    }
    
    // Substitution has no effect and is skipped.
    if (argument.to_string() == variable.to_string()) {
        // The definition is a variable.
        if (dynamic_cast<Variable*>(&*definition)) {
            return
                std::make_unique<Variable>(
                    static_cast<Variable&>(*definition)
                )
            ;
        }
        
        // The definition is an abstraction.
        else if (dynamic_cast<Abstraction*>(&*definition)) {
            return
                std::make_unique<Abstraction>(
                    static_cast<Abstraction&>(*definition)
                )
            ;
        }
        
        // The definition is an application.
        else {
            return
                std::make_unique<Application>(
                    static_cast<Application&>(*definition)
                )
            ;
        }
    }
    
    else {
        return definition->substitute(variable, argument);
    }
}
//}
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

#define V(X) Variable(#X)

// Lambda Term Definitions
//{
// Combinators
//{
/* I Combinator
 * Returns the argument given.
 */
const Abstraction I(
    V(x) >> (
        V(x)
    )
);

/* K Combinator
 * Returns the first argument given and ignores the second.
 */
const Abstraction K(
    V(x) >> (
        V(y) >> (
            V(x)
        )
    )
);

/* S Combinator
 * SK Combinatory Calculus is Turing Complete.
 * S, K, V(x) = I
 */
const Abstraction S(
    V(x) >> (
        V(y) >> (
            V(z) >> (
                V(x), V(z), (V(y), V(z))
            )
        )
    )
);

// B Combinator
const Abstraction B(
    V(x) >> (
        V(y) >> (
            V(z) >> (
                V(x), (V(y), V(z))
            )
        )
    )
);

// C Combinator
const Abstraction C(
    V(x) >> (
        V(y) >> (
            V(z) >> (
                V(x), V(z), V(y)
            )
        )
    )
);

// W Combinator
const Abstraction W(
    V(x) >> (
        V(y) >> (
            V(x), V(y), V(y)
        )
    )
);

/* U Combinator
 * Applies the argument to itself.
 */
const Abstraction U(
    V(x) >> (
        V(x), V(x)
    )
);

/* Y Combinator
 * Allows recursion.
 * Y, V(g) = V(g), (Y, V(g)) = V(g), (V(g), (Y, V(g))) = ...
 */
const Abstraction Y(
    V(g) >> (
        V(x) >> (
            V(g), (V(x), V(x))
        ),
        V(x) >> (
            V(g), (V(x), V(x))
        )
    )
);

/* Iota Combinator
 * The Iota Combinator is Turing Complete by itself.
 * IOTA, IOTA = I
 * IOTA, (IOTA, IOTA) = FALSE = ZERO
 * IOTA, (IOTA, (IOTA, IOTA)) = TRUE = K
 * IOTA, (IOTA, (IOTA, (IOTA, IOTA))) = S
 * SK Combinatory Calculus is Turing Complete, so
 *   Iota Combinatory Calculus is Turing Complete.
 */
const Abstraction IOTA(
    V(f) >> (
        V(f), S, K
    )
);

/* Omega Combinator
 * Has no Beta-Normal Form.
 * Omega reduces to Omega
 */
const Application OMEGA(
    U, U
);
//}

// Booleans
//{
// Boolean True
const Abstraction TRUE(
    V(x) >> (
        V(y) >> (
            V(x)
        )
    )
);

// Boolean False
const Abstraction FALSE(
    V(x) >> (
        V(y) >> (
            V(y)
        )
    )
);

// Boolean Not
const Abstraction NOT(
    V(p) >> (
        V(p), FALSE, TRUE
    )
);

// Boolean And
const Abstraction AND(
    V(p) >> (
        V(q) >> (
            V(p), V(q), V(p)
        )
    )
);

// Boolean Or
const Abstraction OR(
    V(p) >> (
        V(q) >> (
            V(p), V(p), V(q)
        )
    )
);

// Boolean Xor
const Abstraction XOR(
    V(p) >> (
        V(q) >> (
            V(p), (NOT, V(q)), V(q)
        )
    )
);

// Boolean Equality
const Abstraction BEQ(
    V(p) >> (
        V(q) >> (
            V(p), V(q), (NOT, V(q))
        )
    )
);
//}

// Natural Numbers
//{
// Natural Zero
const Abstraction ZERO(
    V(f) >> (
        V(x) >> (
            V(x)
        )
    )
);

// Matural One
const Abstraction ONE(
    V(f) >> (
        V(x) >> (
            V(f), V(x)
        )
    )
);

// Natural Two
const Abstraction TWO(
    V(f) >> (
        V(x) >> (
            V(f), (V(f), V(x))
        )
    )
);

// Natural Successor
const Abstraction SUCC(
    V(n) >> (
        V(f) >> (
            V (x) >> (
                V(f), (V(n), V(f), V(x))
            )
        )
    )
);

// Natural Addition
const Abstraction ADD(
    V(m) >> (
        V(n) >> (
            V(m), SUCC, V(n)
        )
    )
);

// Natural Multiplication
const Abstraction MUL(
    V(m) >> (
        V(n) >> (
            V(m), (ADD, V(n)), ZERO
        )
    )
);

// Natural Exponentiation
const Abstraction EXP(
    V(m) >> (
        V(n) >> (
            V(n), (MUL, V(m)), ONE
        )
    )
);

// Natural Predecessor
const Abstraction PRED(
    V(n) >> (
        V(f) >> (
            V(x) >> (
                V(n),
                (
                    V(g) >> (
                        V(h) >> (
                            V(h), (V(g), V(f))
                        )
                    )
                ),
                (
                    V(u) >> (
                        V(x)
                    )
                ),
                (
                    V(u) >> (
                        V(u)
                    )
                )
            )
        )
    )
);

// Natural Subtraction
const Abstraction SUB(
    V(m) >> (
        V(n) >> (
            V(n), PRED, V(m)
        )
    )
);

// Natural Zero Test
const Abstraction ISZERO(
    V(n) >> (
        V(n),
        (
            V(x) >> (
                FALSE
            )
        ),
        TRUE
    )
);

// Natural Less Than or Equal To Test
const Abstraction LEQ(
    V(m) >> (
        V(n) >> (
            ISZERO, (SUB, V(m), V(n))
        )
    )
);

// Natural Equality Test
const Abstraction EQ(
    V(m) >> (
        V(n) >> (
            AND, (LEQ, V(m), V(n)), (LEQ, V(n), V(m))
        )
    )
);

// Natural Less Than Test
const Abstraction LESS(
    V(m) >> (
        V(n) >> (
            AND, (LEQ, V(m), V(n)), (NOT, (LEQ, V(n), V(m)))
        )
    )
);

// Natural Division
const Abstraction DIV(
    V(x) >> (
        V(y) >> (
            Y,
            (
                V(f) >> (
                    V(n) >> (
                        V(x) >> (
                            V(y) >> (
                                ISZERO, V(x), (PRED, V(n)),
                                (V(f), (SUCC, V(n)), (SUB, V(x), V(y)), V(y))
                            )
                        )
                    )
                )
            ),
            ZERO,
            (SUCC, V(x)),
            V(y)
        )
    )
);

// Natural Modulo
const Application MOD((
    Y,
    (
        V(f) >> (
            V(n) >> (
                V(m) >> (
                    V(x) >> (
                        V(y) >> (
                            LEQ, V(x), V(n), V(m),
                            (
                                V(f),
                                (SUCC, V(n)),
                                (LEQ, V(y), (SUCC, V(m)), ZERO, (SUCC, V(m))),
                                V(x),
                                V(y)
                            )
                        )
                    )
                )
            )
        )
    ),
    ZERO,
    ZERO
));

// Natural Minimum
const Abstraction MIN(
    V(x) >> (
        V(y) >> (
            LEQ, V(x), V(y), V(x), V(y)
        )
    )
);

// Natural Maximum
const Abstraction MAX(
    V(x) >> (
        V(y) >> (
            LEQ, V(x), V(y), V(y), V(x)
        )
    )
);

/**
 * Returns the Church Numeral corresponding to the given argument.
 */
Abstraction NAT(int n) noexcept {
    // The number is positive.
    if (n > 0) {
        Application numeral(V(f), V(x));
        
        for (int i = 1; i < n; ++i) {
            numeral = (V(f), numeral);
        }
        
        return
            Abstraction(
                V(f) >> (
                    V(x) >> (
                        numeral
                    )
                )
            )
        ;
    }
    
    // Requests for a non-positive number returns Church Numeral 0.
    else {
        return ZERO;
    }
}
//}

// Pairs
//{
// Value Pair
const Abstraction PAIR(
    V(x) >> (
        V(y) >> (
            V(f) >> (
                V(f), V(x), V(y)
            )
        )
    )
);

// Pair First Accessor
const Abstraction FIRST(
    V(p) >> (
        V(p), TRUE
    )
);

// Pair Second Accessor
const Abstraction SECOND(
    V(p) >> (
        V(p), FALSE
    )
);
//}

// Lists
//{
// General
//{
// Empty List
const Abstraction NIL(
    V(x) >> (
        TRUE
    )
);

// Empty List Test
const Abstraction ISNIL(
    V(p) >> (
        V(p),
        (
            V(x) >> (
                V(y) >> (
                    FALSE
                )
            )
        )
    )
);

// List Prepending
const Abstraction CONS(
    PAIR
);

// List Head Accessor
const Abstraction HEAD(
    FIRST
);

// List Tail Accessor
const Abstraction TAIL(
    SECOND
);

// List Index Accessor
const Abstraction INDEX(
    V(l) >> (
        V(i) >> (
            HEAD, (V(i), TAIL, V(l))
        )
    )
);

// List Last Accessor
const Application LAST(
    Y,
    (
        V(f) >> (
            V(l) >> (
                ISNIL, (TAIL, V(l)), (HEAD, V(l)), (V(f), (TAIL, V(l)))
            )
        )
    )
);

// List Size
const Application SIZE((
    Y,
    (
        V(f) >> (
            V(n) >> (
                V(l) >> (
                    ISNIL, V(l), V(n), (V(f), (SUCC, V(n)), (TAIL, V(l)))
                )
            )
        )
    ),
    ZERO
));

// List Reverser
const Application REV((
    Y,
    (
        V(f) >> (
            V(n) >> (
                V(l) >> (
                    ISNIL, V(l), V(n), (V(f), (CONS, (HEAD, V(l)), V(n)), (TAIL, V(l)))
                )
            )
        )
    ),
    NIL
));

// Inclusive List Slicer
const Application SLICE((
    Y,
    (
        V(f) >> (
            V(n) >> (
                V(l) >> (
                    V(a) >> (
                        V(b) >> (
                            ISZERO, V(a),
                            (
                                ISZERO, V(b),
                                (REV, (CONS, (HEAD, V(l)), V(n))),
                                (V(f), (CONS, (HEAD, V(l)), V(n)), (TAIL, V(l)), V(a), (PRED, V(b)))
                            ),
                            (
                                V(f), V(n), (V(a), TAIL, V(l)), ZERO, (SUB, V(b), V(a))
                            )
                        )
                    )
                )
            )
        )
    ),
    NIL
));

// List Appending
const Abstraction APP(
    V(l) >> (
        V(x) >> (
            REV, (CONS, V(x), (REV, V(l)))
        )
    )
);

// List Concatenation
const Abstraction CONCAT(
    V(x) >> (
        V(y) >> (
            Y,
            (
                V(f) >> (
                    V(x) >> (
                        V(y) >> (
                            ISNIL, V(x), V(y), (V(f), (TAIL, V(x)), (CONS, (HEAD, V(x)), V(y)))
                        )
                    )
                )
            ),
            (REV, V(x)),
            V(y)
        )
    )
);

// List Function Mapping
const Abstraction MAP(
    V(l) >> (
        V(g) >> (
            Y,
            (
                V(f) >> (
                    V(n) >> (
                        V(l) >> (
                            V(g) >> (
                                ISNIL, V(l), V(n),
                                (V(f), (CONS, (V(g), (HEAD, V(l))), V(n)), (TAIL, V(l)), V(g))
                            )
                        )
                    )
                )
            ),
            NIL,
            (REV, V(l)),
            V(g)
        )
    )
);

// List Function Folding
const Abstraction FOLD(
    V(l) >> (
        V(g) >> (
            Y,
            (
                V(f) >> (
                    V(l) >> (
                        V(n) >> (
                            V(g) >> (
                                ISNIL, V(l), V(n),
                                (V(f), (TAIL, V(l)), (V(g), (HEAD, V(l)), V(n)), V(g))
                            )
                        )
                    )
                )
            ),
            (TAIL, V(l)),
            (HEAD, V(l)),
            V(g)
        )
    )
);

// List Function Filtering
const Abstraction FILT(
    V(l) >> (
        V(g) >> (
            Y,
            (
                V(f) >> (
                    V(n) >> (
                        V(l) >> (
                            V(g) >> (
                                ISNIL, V(l), V(n),
                                (
                                    V(f),
                                    (
                                        V(g), (HEAD, V(l)),
                                        (CONS, (HEAD, V(l)), V(n)),
                                        V(n)
                                    ),
                                    (TAIL, V(l)),
                                    V(g)
                                )
                            )
                        )
                    )
                )
            ),
            NIL,
            (REV, V(l)),
            V(g)
        )
    )
);
//}

// Generation
//{
// Duplicate List
const Abstraction DUP(
    V(x) >> (
        V(n) >> (
            V(n), (CONS, V(x)), NIL
        )
    )
);

// Inclusive Descending List
const Application DLIST(
    Y,
    (
        V(f) >> (
            V(n) >> (
                CONS, V(n), (ISZERO, V(n), NIL, (V(f), (PRED, V(n))))
            )
        )
    )
);

// Inclusive Ascending List
const Application ALIST((
    Y,
    (
        V(f) >> (
            V(i) >> (
                V(n) >> (
                    CONS, V(i), (LEQ, V(n), V(i), NIL, (V(f), (SUCC, V(i)), V(n)))
                )
            )
        )
    ),
    ZERO
));

// Inclusive Range
const Application RANGE(
    Y,
    (
        V(f) >> (
            V(a) >> (
                V(b) >> (
                    CONS, V(a),
                    (
                        LEQ, V(a), V(b),
                        (
                            LEQ, V(b), V(a),
                            NIL,
                            (V(f), (SUCC, V(a)), V(b))
                        ),
                        (
                            V(f), (PRED, V(a)), V(b)
                        )
                    )
                )
            )
        )
    )
);
//}

// Algorithms
//{
// List Boolean AND
const Application LAND(
    Y,
    (
        V(f) >> (
            V(l) >> (
                ISNIL, V(l), TRUE, (HEAD, V(l), (V(f), (TAIL, V(l))), FALSE)
            )
        )
    )
);

// List Boolean OR
const Application LOR(
    Y,
    (
        V(f) >> (
            V(l) >> (
                ISNIL, V(l), FALSE, (HEAD, V(l), TRUE, (V(f), (TAIL, V(l))))
            )
        )
    )
);

// List Natural Minimum
const Abstraction LMIN(
    V(l) >> (
        Y,
        (
            V(f) >> (
                V(l) >> (
                    V(n) >> (
                        ISNIL, V(l), V(n), (V(f), (TAIL, V(l)), (MIN, (HEAD, V(l)), V(n)))
                    )
                )
            )
        ),
        (TAIL, V(l)),
        (HEAD, V(l))
    )
);

// List Natural Maximum
const Abstraction LMAX(
    V(l) >> (
        Y,
        (
            V(f) >> (
                V(l) >> (
                    V(n) >> (
                        ISNIL, V(l), V(n), (V(f), (TAIL, V(l)), (MAX, (HEAD, V(l)), V(n)))
                    )
                )
            )
        ),
        (TAIL, V(l)),
        (HEAD, V(l))
    )
);

// List Natural Addition
const Abstraction LADD(
    V(l) >> (
        Y,
        (
            V(f) >> (
                V(l) >> (
                    V(n) >> (
                        ISNIL, V(l), V(n), (V(f), (TAIL, V(l)), (ADD, (HEAD, V(l)), V(n)))
                    )
                )
            )
        ),
        (TAIL, V(l)),
        (HEAD, V(l))
    )
);

// List Natural Multiplication
const Abstraction LMUL(
    V(l) >> (
        Y,
        (
            V(f) >> (
                V(l) >> (
                    V(n) >> (
                        ISNIL, V(l), V(n), (V(f), (TAIL, V(l)), (MUL, (HEAD, V(l)), V(n)))
                    )
                )
            )
        ),
        (TAIL, V(l)),
        (HEAD, V(l))
    )
);
//}

// Alternative Cons Syntax
//{
/**
 * Takes two arguments and applies the Cons (Pair) function to them.
 */
template <typename Element, typename List>
Application operator|(const Element& element, const List& list) noexcept {
    return CONS, element, list;
}
//}
//}

// Trees
//{
// General
//{
// Tree Formation
const Abstraction TREE(
    V(v) >> (
        V(l) >> (
            V(r) >> (
                V(f) >> (
                    V(f), V(v), V(l), V(r)
                )
            )
        )
    )
);

// Empty Tree
const Abstraction ENODE(
    NIL
);

// Empty Tree Test
const Abstraction ISEXT(
    V(t) >> (
        V(t),
        (
            V(v) >> (
                V(l) >> (
                    V(r) >> (
                        FALSE
                    )
                )
            )
        )
    )
);

// Tree Value Extraction
const Abstraction TVAL(
    V(t) >> (
        V(t),
        (
            V(v) >> (
                V(l) >> (
                    V(r) >> (
                        V(v)
                    )
                )
            )
        )
    )
);

// Left Tree Extraction
const Abstraction LTREE(
    V(t) >> (
        V(t),
        (
            V(v) >> (
                V(l) >> (
                    V(r) >> (
                        V(l)
                    )
                )
            )
        )
    )
);

// Right Tree Extraction
const Abstraction RTREE(
    V(t) >> (
        V(t),
        (
            V(v) >> (
                V(l) >> (
                    V(r) >> (
                        V(r)
                    )
                )
            )
        )
    )
);
//}
//}

// Integers
//{
// Postive Sign
const Abstraction PVE(
    TRUE
);

// Negative Sign
const Abstraction NVE(
    FALSE
);

// Integer Sign Extraction
const Abstraction SIGN(
    FIRST
);

// Integer Sign Equality Test
const Abstraction SEQ(
    V(x) >> (
        V(y) >> (
            BEQ, (SIGN, V(x)), (SIGN, V(y))
        )
    )
);

// Integer Magnitude Extraction
const Abstraction MAG(
    SECOND
);

// Integer Magnitude Equality Test
const Abstraction MEQ(
    V(x) >> (
        V(y) >> (
            EQ, (MAG, V(x)), (MAG, V(y))
        )
    )
);

// Integer Sign Swap
const Abstraction SWAP(
    V(i) >> (
        PAIR, (NOT, (SIGN, V(i))), (MAG, V(i))
    )
);

// Integer Positive Zero
const Application IZEROP((
    PAIR, PVE, ZERO
));

// Integer Negative Zero
const Application IZERON((
    PAIR, NVE, ZERO
));

// Integer Zero Test
const Abstraction ISIZERO(
    V(i) >> (
        ISZERO, (MAG, V(i))
    )
);

// Integer Equality Test
const Abstraction IEQ(
    V(x) >> (
        V(y) >> (
            AND, (ISIZERO, V(x)), (ISIZERO, V(y)),
            (
                TRUE
            ),
            (
                AND, (SEQ, V(x), V(y)), (MEQ, V(x), V(y))
            )
        )
    )
);

// Integer Postive One
const Application IONEP((
    PAIR, PVE, ONE
));

// Integer Negative One
const Application IONEN((
    PAIR, NVE, ONE
));

// Integer Postive Two
const Application ITWOP((
    PAIR, PVE, TWO
));

// Integer Negative Two
const Application ITWON((
    PAIR, NVE, TWO
));

// Integer Positive Test
const Abstraction ISPOS(
    V(i) >> (
        ISIZERO, V(i),
        (
            FALSE
        ),
        (
            SIGN, V(i)
        )
    )
);

// Integer Negative Test
const Abstraction ISNEG(
    V(i) >> (
        ISIZERO, V(i),
        (
            FALSE
        ),
        (
            NOT, (SIGN, V(i))
        )
    )
);

// Integer Increment
const Abstraction INC(
    V(i) >> (
        SIGN, V(i),
        (
            PAIR, (SIGN, V(i)), (SUCC, (MAG, V(i)))
        ),
        (
            ISIZERO, V(i),
            (
                IONEP
            ),
            (
                PAIR, (SIGN, V(i)), (PRED, (MAG, V(i)))
            )
        )
    )
);

// Integer Decrement
const Abstraction DEC(
    V(i) >> (
        SIGN, V(i),
        (
            ISIZERO, V(i),
            (
                IONEN
            ),
            (
                PAIR, (SIGN, V(i)), (PRED, (MAG, V(i)))
            )
        ),
        (
            PAIR, (SIGN, V(i)), (SUCC, (MAG, V(i)))
        )
    )
);

// Integer Addition
const Abstraction IADD(
    V(x) >> (
        V(y) >> (
            PAIR,
            (
                SEQ, V(x), V(y),
                (
                    (SIGN, V(x)), (ADD, (MAG, V(x)), (MAG, V(y)))
                ),
                (
                    LEQ, (MAG, V(x)), (MAG, V(y)),
                    (
                        (SIGN, V(y)), (SUB, (MAG, V(y)), (MAG, V(x)))
                    ),
                    (
                        (SIGN, V(x)), (SUB, (MAG, V(x)), (MAG, V(y)))
                    )
                )
            )
        )
    )
);

// Integer Substraction
const Abstraction ISUB(
    V(x) >> (
        V(y) >> (
            IADD, V(x), (SWAP, V(y))
        )
    )
);

// Integer Multiplication
const Abstraction IMUL(
    V(x) >> (
        V(y) >> (
            PAIR, (SEQ, V(x), V(y)), (MUL, (MAG, V(x)), (MAG, V(y)))
        )
    )
);

// Integer Division
const Abstraction IDIV(
    V(x) >> (
        V(y) >> (
            PAIR, (SEQ, V(x), V(y)), (DIV, (MAG, V(x)), (MAG, V(y)))
        )
    )
);

// Integer Modulo
const Abstraction IMOD(
    V(x) >> (
        V(y) >> (
            PAIR, (SIGN, V(y)),
            (
                SEQ, V(x), V(y),
                (
                    MOD, (MAG, V(x)), (MAG, V(y))
                ),
                (
                    MOD, (SUB, (MAG, V(y)), (MOD, (MAG, V(x)), (MAG, V(y)))), (MAG, V(y))
                )
            )
        )
    )
);

// Integer Exponential
const Abstraction IEXP(
    V(x) >> (
        V(y) >> (
            ISNEG, V(y),
            (
                IZEROP
            ),
            (
                PAIR,
                (
                    SIGN, V(x),
                    (
                        PVE
                    ),
                    (
                        ISIZERO, (IMOD, V(y), ITWOP)
                    )
                ),
                (
                    EXP, (MAG, V(x)), (MAG, V(y))
                )
            )
        )
    )
);

/**
 * Returns the integer corresponding to the given argument.
 */
Application INT(int i) noexcept {
    return PAIR, (i < 0 ? NVE : PVE), NAT(std::abs(i));
}
//}

// Algorithms
//{
// Factorial Function
const Application FACT(
    Y,
    (
        V(f) >> (
            V(n) >> (
                ISZERO, V(n), ONE, (MUL, V(n), (V(f), (PRED, V(n))))
            )
        )
    )
);

// Fibonacci Function
const Application FIBO(
    Y,
    (
        V(f) >> (
            V(n) >> (
                ISZERO, V(n), ZERO,
                (
                    ISZERO, (PRED, V(n)), ONE,
                    (ADD, (V(f), (PRED, V(n))), (V(f), (PRED, (PRED, V(n)))))
                )
            )
        )
    )
);
//}
//}

#undef V
#ifndef SOURCEMODEL__TOGGLE_PARAMETER_H
#define SOURCEMODEL__TOGGLE_PARAMETER_H

#include <sigslot/signal.hpp>
#include <string>

#include "math/utils.h"

class ToggleParameter {
   public:
    ToggleParameter(const std::string& name, bool initial);

    const std::string& name() const;

    bool value() const;
    void setValue(bool);

    sigslot::signal<const std::string&, Scalar>
        valueChanged;  // still use Scalar for (false = 0, true = 1)

   private:
    std::string m_name;
    bool        m_value;
};

#endif  //  SOURCEMODEL__TOGGLE_PARAMETER_H

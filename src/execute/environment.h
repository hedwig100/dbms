#ifndef _EXECUTE_ENVIRONMENT_H
#define _EXECUTE_ENVIRONMENT_H

#include "metadata.h"

namespace execute {

class Environment {
  public:
    Environment() { table_manager_ = metadata::TableManager(); }

    const metadata::TableManager &GetTableManager() const {
        return table_manager_;
    }

  private:
    metadata::TableManager table_manager_;
};

} // namespace execute

#endif
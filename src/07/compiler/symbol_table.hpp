#ifndef H_SYMBOL_TABLE_H
#define H_SYMBOL_TABLE_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace compiler
{
    using SymbolScope = std::string;

    namespace SymbolScopeType
    {
        const SymbolScope GlobalScope = "GLOBAL";
    }

    struct Symbol{
        std::string Name;
        SymbolScope Scope;
        int Index;

        Symbol(std::string name, SymbolScope scope, int idx): Name(name), Scope(scope), Index(idx){}

        bool operator==(const Symbol &rhs) const
		{
			return (Name == rhs.Name && Scope == rhs.Scope && Index == rhs.Index);
		}

        bool operator!=(const Symbol &rhs) const
		{
			return (Name != rhs.Name || Scope != rhs.Scope || Index != rhs.Index);
		}
    };

    struct SymbolTable{
        std::map<std::string, std::shared_ptr<Symbol>> store;
        int numDefinitions;

        std::shared_ptr<Symbol> Define(std::string name)
        {
            auto symbol = std::make_shared<Symbol>(name, SymbolScopeType::GlobalScope, numDefinitions);
            store[name] = symbol;
            numDefinitions++;
            return symbol;
        }

        std::shared_ptr<Symbol> Resolve(std::string name)
        {
            auto fit = store.find(name);
            if(fit != store.end())
            {
                return fit->second;
            }
            else
            {
                return nullptr;
            }
        }
    };

    std::shared_ptr<SymbolTable> NewSymbolTable()
    {
        return std::make_shared<SymbolTable>();
    }
}

#endif // H_SYMBOL_TABLE_H
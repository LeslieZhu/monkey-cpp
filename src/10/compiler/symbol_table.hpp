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
        const SymbolScope LocalScope = "LOCAL";
        const SymbolScope BuiltinScope = "BUILTIN";
        const SymbolScope FreeScope = "FREE";
        const SymbolScope FunctionScope = "FUNCTION";
    }

    struct Symbol{
        std::string Name;
        SymbolScope Scope;
        int Index;

        Symbol(std::string name, int idx): Name(name), Index(idx){}
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
        std::shared_ptr<SymbolTable> Outer;
        std::map<std::string, std::shared_ptr<Symbol>> store;
        int numDefinitions;
        std::vector<std::shared_ptr<Symbol>> FreeSymbols;

        std::shared_ptr<Symbol> Define(std::string name)
        {
            auto symbol = std::make_shared<Symbol>(name, numDefinitions);

            if(Outer == nullptr)
            {
                symbol->Scope = SymbolScopeType::GlobalScope;
            } else {
                symbol->Scope = SymbolScopeType::LocalScope;
            }

            store[name] = symbol;
            numDefinitions++;
            return symbol;
        }

        std::shared_ptr<Symbol> DefineBuiltin(const int& index, std::string name)
        {
            auto symbol = std::make_shared<Symbol>(name, SymbolScopeType::BuiltinScope ,index);
            store[name] = symbol;
            return symbol;
        }

        std::shared_ptr<Symbol> DefineFree(std::shared_ptr<Symbol> original)
        {
            FreeSymbols.push_back(original);

            auto symbol = std::make_shared<Symbol>(original->Name, SymbolScopeType::FreeScope ,FreeSymbols.size() - 1);
            store[original->Name] = symbol;
            return symbol;
        }

        std::shared_ptr<Symbol> DefineFunctionName(std::string name)
        {
            auto symbol = std::make_shared<Symbol>(name, SymbolScopeType::FunctionScope ,0);
            store[name] = symbol;
            return symbol;
        }

        std::shared_ptr<Symbol> Resolve(std::string name)
        {
            auto fit = store.find(name);
            if(fit != store.end())
            {
                return fit->second;
            }
            else if(Outer != nullptr)
            {
                auto obj = Outer->Resolve(name);
                if(obj == nullptr)
                {
                    return obj;
                }

                if(obj->Scope == SymbolScopeType::GlobalScope || obj->Scope == SymbolScopeType::BuiltinScope)
                {
                    return obj;
                }

                auto free = DefineFree(obj);
                return free;
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

    std::shared_ptr<SymbolTable> NewEnclosedSymbolTable(std::shared_ptr<SymbolTable> outer)
    {
        auto symbol = std::make_shared<SymbolTable>();
        symbol->Outer = outer;
        return symbol;
    }
}

#endif // H_SYMBOL_TABLE_H
#include "Token.h"

namespace srdo {
    std::ostream& operator<<(std::ostream& os, const TType& tt)
    {
        static std::unordered_map<TType, const char*> strings;
        if(strings.size() == 0)
        {
            strings.reserve(static_cast<size_t>(TType::Eof) + 1);
            /* because C++ doesn't let you print out the name of an enum by default,
             * I have use a map to do that.
             * That means, however, that I have to do this.
             * I've softened the blow a bit by using this macro, but it's not the best.
             * I hope to eventually find a better way to do this.
             */
            #define INSERT_ELEMENT(x) strings[TType::x] = #x
            INSERT_ELEMENT(Unknown);

            INSERT_ELEMENT(Keyword);
            INSERT_ELEMENT(Id);

            INSERT_ELEMENT(Int);
            INSERT_ELEMENT(Float);
            INSERT_ELEMENT(Double);
            INSERT_ELEMENT(Bool);

            INSERT_ELEMENT(Plus);
            INSERT_ELEMENT(Minus);
            INSERT_ELEMENT(Mul);
            INSERT_ELEMENT(Div);
            INSERT_ELEMENT(Pow);
            INSERT_ELEMENT(Assign);
            
            INSERT_ELEMENT(Not);        INSERT_ELEMENT(BitNot); 
            INSERT_ELEMENT(And);        INSERT_ELEMENT(BitAnd);
            INSERT_ELEMENT(Or);         INSERT_ELEMENT(BitOr);
            INSERT_ELEMENT(Xor);        INSERT_ELEMENT(BitXor);
            INSERT_ELEMENT(BitShiftL);  INSERT_ELEMENT(BitShiftR);

            INSERT_ELEMENT(LParen);     INSERT_ELEMENT(RParen);
            INSERT_ELEMENT(LAngle);     INSERT_ELEMENT(RAngle);
            INSERT_ELEMENT(LCurly);     INSERT_ELEMENT(RCurly);
            INSERT_ELEMENT(LSquare);    INSERT_ELEMENT(RSquare);

            INSERT_ELEMENT(Eq);         INSERT_ELEMENT(Ne);
            INSERT_ELEMENT(Gte);        INSERT_ELEMENT(Lte);

            INSERT_ELEMENT(Terminal);   INSERT_ELEMENT(Eof);

            #undef INSERT_ELEMENT
        }
        return os << strings[tt];
    }

    std::ostream& operator<<(std::ostream& os, const Token& tok)
    {
        if(tok.value == "")
        {
            return os << tok.type << ":" << tok.value;
        }
        return os << tok.type;
    }

    std::ostream& operator<<(std::ostream& os, const std::vector<Token>& tokens)
    {
        os << "[";
        for(int i = 0; i < tokens.size(); i++)
        {
            os << tokens[i];
            if(i < tokens.size() - 1)
            {
                os << ", ";
            }
        }
        os << "]";
        return os;
    }
}
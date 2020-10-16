#pragma once

#include "../Datatypes/Position.h"

namespace srdo {
    class Issue
    {
    public:
        Issue(const std::string& issueType, const std::string& message, const Position& pos)
            : issueType(issueType), message(message), pos(pos)
        {
        }

        std::string issueType;
        std::string message;
        Position pos;

        virtual void logIssue()
        {
            std::cout << issueType << " " << message << "\n" 
                    << "File '" << pos.fn << "' at " << pos.ln + 1 << ", " << pos.col + 1 << "\n";
        }
    };

    // To be used when there is a problem, but the program can still compile
    class Warning : public Issue
    {
    public:
        Warning(const std::string& message, const Position& pos)
            : Issue("\x1b[33m" "[Warning]" "\x1b[0m", message, pos)
        {
        }
    };

    // To be used when the program can't compile but the compiler will keep going
    // to find more errors
    class Error : public Issue
    {
    public:
        Error(const std::string& message, const Position& pos)
            : Issue("\x1b[31m" "[Warning]" "\x1b[0m", message, pos)
        {
        }
    };

    // To be used when compiling can't go on any longer
    class Fatal : public Issue
    {
    public:
        Fatal(const std::string& message)
            : Issue("\x1b[31m" "[Fatal]" "\x1b[0m", message, {"", "", 0, 0, 0})
        {
        }

        virtual void logIssue() final
        {
            std::cout << issueType << message << "\n";
        }
    };

    template<typename T>
    class IssuePack
    {
    public:
        IssuePack(const std::string& fatalMessage, int maxIssueCount)
            : fatalMessage(fatalMessage), maxIssueCount(maxIssueCount)
        {
            issues.reserve(maxIssueCount);
        }

        template<typename ... Args>
        void emplaceIssue(Args&&... args)
        {
            issueCount++;
            if(issueCount >= maxIssueCount)
            {
                fatalIssue = Fatal(fatalMessage);
                return;
            }
            issues.emplace_back(std::forward<Args>(args)...);
        }

        constexpr bool isFatal() const
        {
            return fatalIssue.has_value();
        }

        void logIssues() const
        {
            for(auto issue : issues)
            {
                issue.logIssue();
            }
        }
    private:
        std::string fatalMessage;
        std::vector<T> issues;
        std::optional<Fatal> fatalIssue;
        uint32_t issueCount = 0;
        const int maxIssueCount;
    };
}
#pragma once


#include "my_referense_wrapper.h"
#include "void_traits.h"
#include "my_functional.h"
#include "my_tuple.h"


#include <iostream>
#include <vector>
#include <memory>



class MyCycleException : public std::exception {
public:
    MyCycleException() = default;

    const char *what() const noexcept override {
        return "Trying to create a cycle.";
    }
};


class TypeId {
private:
    size_t id_;

    TypeId() = default;

    TypeId(size_t id) : id_(id) {
    }

public:
    friend class TTaskScheduler;

    TypeId(const TypeId &) = default;

    TypeId &operator=(const TypeId &) = default;

    operator size_t() const {
        return id_;
    }
};


class TTaskScheduler {
private:
    template<typename T>
    struct Result {
        TTaskScheduler &task_scheduler_;

        size_t id_;

        Result(size_t id, TTaskScheduler &task_scheduler) : id_(id), task_scheduler_(task_scheduler) {
        }

        operator T() {
            return task_scheduler_.tasks_[id_].get_current_task()->get_result()->get_result<T>();
        }
    };


    class GetResult {
    private:
        class ResultBase {
        public:
            virtual ~ResultBase() = default;
        };

        template<typename T>
        class CurrentResult : public ResultBase {
        private:
            T value_;

        public:
            ~CurrentResult() override = default;

            CurrentResult(T &&value) : value_(std::forward<T>(value)) {
            }

            CurrentResult(const CurrentResult &other) = delete;

            CurrentResult &operator=(const CurrentResult &other) = delete;

            T get_result() {
                return value_;
            }
        };

        std::shared_ptr<ResultBase> result_;

    public:
        GetResult() = default;

        template<typename T>
        GetResult(T &&value) : result_(std::make_shared<CurrentResult<T> >(std::forward<T>(value))) {
        }

        template<typename T>
        T get_result() {
            auto *ptr = dynamic_cast<CurrentResult<T> *>(result_.get());
            if (!ptr) {
                throw std::bad_cast();
            }
            return ptr->get_result();
        }
    };

    class Task {
    private:
        class TaskBase {
        protected:
            bool executed_ = false;

        public:
            virtual ~TaskBase() = default;

            virtual std::shared_ptr<GetResult> get_result() = 0;

            virtual void execute() = 0;

            bool is_executed() const {
                return executed_;
            }
        };


        template<typename Func, typename... Args>
        class TaskWithArgs : public TaskBase {
        public:
            TaskWithArgs(Func &&func, Args &&... args)
                : func_(std::forward<Func>(func)), args_(std::forward<Args>(args)...) {
            }

            void execute() override {
                result_ = std::make_shared<GetResult>(
                    my_tuple_apply(std::forward<Func>(func_), std::forward<MyTuple<Args...> >(args_)));
                this->executed_ = true;
            }

            std::shared_ptr<GetResult> get_result() override {
                return result_;
            }

        private:
            Func func_;
            MyTuple<Args...> args_;
            std::shared_ptr<GetResult> result_;
        };


        std::shared_ptr<TaskBase> current_task_;

    public:
        std::shared_ptr<TaskBase> get_current_task() {
            return current_task_;
        }

        template<typename Func, typename... Args>
        Task(Func &&func, Args &&... args) {
            current_task_ = std::make_shared<TaskWithArgs<Func, Args...> >(
                std::forward<Func>(func), std::forward<Args>(args)...);
        }
    };


    void dfs(size_t id) {
        colors_[id] = 1;
        for (auto prev_id: graph_[id]) {
            if (colors_[prev_id] == 0) {
                dfs(prev_id);
            }
            if (colors_[prev_id] == 1) {
                throw MyCycleException();
            }
        }
        auto current = tasks_[id].get_current_task();
        if (!current->is_executed()) {
            current->execute();
        }
        colors_[id] = 2;
    }


    std::vector<Task> tasks_;
    size_t current_id_ = -1;

    std::vector<std::vector<TypeId> > graph_;
    std::vector<int> colors_;

public:
    template<typename T>
    Result<typename my_remove_reference<T>::type> getFutureResult(TypeId id) {

        using type = my_remove_reference<T>::type;
        
        if (id >= tasks_.size()) {
            throw MyCycleException();
        }
        if (graph_.size() == current_id_ + 1) {
            graph_.emplace_back(std::vector<TypeId>());
        }
        graph_[current_id_ + 1].push_back(id);
        return Result<type>(id, *this);
    }


    template<typename Func, typename... Args>
    TypeId add(Func &&func, Args &&... args) {
        using return_type = decltype(my_invoke(std::forward<Func>(func), std::forward<Args>(args)...));

        static_assert(!is_void_type<return_type>::value, "Cannot add function that returns void.");

        tasks_.push_back(Task(std::forward<Func>(func), std::forward<Args>(args)...));
        if (graph_.size() == current_id_ + 1) {
            graph_.emplace_back(std::vector<TypeId>());
        }
        return ++current_id_;
    }


    void executeAll() {
        for (auto &task: tasks_) {
            auto current = task.get_current_task();
            if (!current->is_executed()) {
                current->execute();
            }
        }
    }


    template<typename T>
    my_remove_reference<T>::type getResult(size_t id) {

        using type = my_remove_reference<T>::type;

        auto current = tasks_[id].get_current_task();
        if (!current->is_executed()) {
            colors_.resize(current_id_ + 1, 0);
            try {
                dfs(id);
            } catch (...) {
                throw;
            }
            colors_.clear();
        }
        return current->get_result()->get_result<type>();
    }
};
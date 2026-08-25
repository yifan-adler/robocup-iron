/*
 * File: rdfw.cpp
 * Author : ShiQiao Chen(陈世侨)
 * Affiliation: WuHan University of Technology
 */
 #include <numeric>
 #include <unordered_map>
 #include <set>
 #include <map>
# include <cstring>
# include <iostream>
# include <regex>
# include "rdfw.hpp"
# include "parser.hpp"
using namespace _home;
using namespace std;

void split_string(vector<string> &out, const string &str_source, char mark);
ostream &operator<<(ostream &os, shared_ptr<Object> obj);
ostream &operator<<(ostream &os, shared_ptr<SyntaxNode> sn);
ostream &operator<<(ostream &os, const Instruction &instr);


/**
 * Load the team name.
 */
RDFW::RDFW() : Plug("RDFW") {
    
}

/**
 * @brief 初始化动态数组
 * @param max_size 数组的最大大小，默认为100
 */
void RDFW::InitializeDynamicArrays(int max_size) {
    cout << "#(RDFW): Initializing dynamic arrays with size " << max_size << endl;
    
    // 初始化一维数组 - 使用reserve优化内存分配
    goto_cons.clear();
    goto_cons.reserve(max_size);
    goto_cons.resize(max_size, 0);
    
    putdown1_cons.clear();
    putdown1_cons.reserve(max_size);
    putdown1_cons.resize(max_size, 0);
    
    open_cons.clear();
    open_cons.reserve(max_size);
    open_cons.resize(max_size, 0);
    
    close_cons.clear();
    close_cons.reserve(max_size);
    close_cons.resize(max_size, 0);
    
    pickup_cons.clear();
    pickup_cons.reserve(max_size);
    pickup_cons.resize(max_size, 0);
    
    givehuman_cons.clear();
    givehuman_cons.reserve(max_size);
    givehuman_cons.resize(max_size, 0);
    
    fromplate_cons.clear();
    fromplate_cons.reserve(max_size);
    fromplate_cons.resize(max_size, 0);
    
    toplate_cons.clear();
    toplate_cons.reserve(max_size);
    toplate_cons.resize(max_size, 0);
    
    rightlocation.clear();
    rightlocation.reserve(max_size);
    rightlocation.resize(max_size, false);
    
    // 初始化二维数组 - 优化内存分配
    putin_cons.clear();
    putin_cons.reserve(max_size);
    for (int i = 0; i < max_size; i++) {
        putin_cons.emplace_back(max_size, 0);
    }
    
    takeout_cons.clear();
    takeout_cons.reserve(max_size);
    for (int i = 0; i < max_size; i++) {
        takeout_cons.emplace_back(max_size, 0);
    }
    
    putdown_cons.clear();
    putdown_cons.reserve(max_size);
    for (int i = 0; i < max_size; i++) {
        putdown_cons.emplace_back(max_size, 0);
    }
    
    move_cons.clear();
    move_cons.reserve(max_size);
    for (int i = 0; i < max_size; i++) {
        move_cons.emplace_back(max_size, 0);
    }
    
    mustnear_cons.clear();
    mustnear_cons.reserve(max_size);
    for (int i = 0; i < max_size; i++) {
        mustnear_cons.emplace_back(max_size, 0);
    }
    
    // 初始化任务查找表 - 优化内存分配
    takeout.clear();
    takeout.reserve(max_size);
    for (int i = 0; i < max_size; i++) {
        takeout.emplace_back(max_size, nullptr);
    }
    
    putin.clear();
    putin.reserve(max_size);
    for (int i = 0; i < max_size; i++) {
        putin.emplace_back(max_size, nullptr);
    }
    
    close.clear();
    close.reserve(max_size);
    close.resize(max_size, nullptr);
    
    open.clear();
    open.reserve(max_size);
    open.resize(max_size, nullptr);
    
    pickup.clear();
    pickup.reserve(max_size);
    pickup.resize(max_size, nullptr);
    
    putdown.clear();
    putdown.reserve(max_size);
    putdown.resize(max_size, nullptr);
    
    cout << "#(RDFW): Dynamic arrays initialization completed" << endl;
}


/**
 * @brief Initialization Function
 * @param argc:     argument count
 * @param argv:     argument value
 * Tasks:
 * 1. Parse Arguments and set options --> LOG
 * 2. 
 */
void RDFW::Init(int argc, char **argv) // 改
{
    string path = "../example/words.txt";   // loading word dictionary
    if (argc > 1)
    {
        // Task 1: Parse Arguments and set options --> LOG
        bool optionFound = false; // 记录是否找到匹配的选项

        for (int i = 1; i < argc; i++)
        {
            LOG("%s", argv[i]);

            const std::string arg(argv[i]);
            const std::string nextArg(i + 1 < argc ? argv[i + 1] : "");

            // Change Option
            enum Options
            {
                NLP,
                ERR,
                ASK_2,
                PATH,
                STAGE
            };

            int option = -1;
            if (arg == "-nlp"){
                option = NLP;
                optionFound = true;
            } else if (arg == "-err") {
                option = ERR;
                optionFound = true;
            } else if (arg == "-ask_2"){
                option = ASK_2;
                optionFound = true;
            } else if (arg == "-path") {
                option = PATH;
                optionFound = true;
            } else if (arg == "-stage") {
                option = STAGE;
                optionFound = true;
            }

            // Set Option Value (Option value is in the next arg.)
            switch (option)
            {
                case NLP:
                    isNaturalParse = stoi(nextArg); // Extract next arg's value (e.g.  -nlp 0)
                    i++;
                    break;
                case ERR:
                    isErrorCorrection = stoi(nextArg);
                    i++;
                    break;
                case ASK_2:
                    isAskTwice = stoi(nextArg);
                    i++;
                    break;
                case PATH:
                    path = nextArg;
                    i++;
                    break;
                case STAGE:
                {
                    int temp = stoi(nextArg); 
                    if(temp==1) stage=1;
                    if(temp==2) stage=2;
                    i++;
                }
                break;
            }
        }

        if (!optionFound)
        {
            LOG("notfound"); // 处理未匹配的选项
            // 可以输出错误信息或执行其他操作
        }
    }
    LOG("nlp %d, err %d", isNaturalParse, isErrorCorrection);

    objects.push_back(shared_from_this());  // 向物品中添加当前对象

    if (isErrorCorrection)
        posCorrectFlag.push_back(false);
    else
        posCorrectFlag.push_back(true);
    
    // 初始化动态数组
    InitializeDynamicArrays(100);  // 设置为100以支持更大的位置索引
    
    // 初始化位置感知记录数组，为所有可能的位置预留空间
    posSensedFlag.resize(100, false);  // 为100个位置预留空间，全部标记为未感知
    
    // 初始化位置感知物体记录数组
    locationSensedObjects.resize(100);  // 为100个位置预留空间

    nlp_parser = new parser();
    nlp_parser->words_map_initialize(path);
}


void RDFW::Plan() // 改
{   
    // ==================== 测试开始前的状态验证 ====================
    cout << "#(RDFW): Starting new test - verifying clean state" << endl;
    
    // 验证关键状态变量是否已重置
    SetSolvedTaskNum(0);
    task_index = 0;
    err_times = 0;
    isPass = false;
    isKeepConstrain = false;
    isMultiGotoMode = false;
    isAutoConstrain = false;
    
    // 验证机器人状态
    location = UNKNOWN;
    hold = nullptr;
    hold_id = 0;
    
    // 验证感知状态
    for (size_t i = 0; i < posSensedFlag.size(); i++) {
        posSensedFlag[i] = false;
    }
    
    // 验证位置感知物体记录
    for (auto& loc_info : locationSensedObjects) {
        loc_info.object_ids.clear();
        loc_info.container_id = 0;
        loc_info.has_container = false;
    }
    
    // 验证约束查找表
    lock_by_mustnear.clear();
    
    // 验证并查集状态
    memset(uf_parent, -1, sizeof(uf_parent));
    memset(uf_size, 0, sizeof(uf_size));
    memset(uf_groupLoc, -1, sizeof(uf_groupLoc));
    
    // 验证动态数组状态
    fill(goto_cons.begin(), goto_cons.end(), 0);
    fill(putdown1_cons.begin(), putdown1_cons.end(), 0);
    fill(open_cons.begin(), open_cons.end(), 0);
    fill(close_cons.begin(), close_cons.end(), 0);
    fill(pickup_cons.begin(), pickup_cons.end(), 0);
    fill(givehuman_cons.begin(), givehuman_cons.end(), 0);
    fill(fromplate_cons.begin(), fromplate_cons.end(), 0);
    fill(toplate_cons.begin(), toplate_cons.end(), 0);
    fill(rightlocation.begin(), rightlocation.end(), false);
    
    // 验证二维数组状态
    for (auto& row : putin_cons) fill(row.begin(), row.end(), 0);
    for (auto& row : takeout_cons) fill(row.begin(), row.end(), 0);
    for (auto& row : putdown_cons) fill(row.begin(), row.end(), 0);
    for (auto& row : move_cons) fill(row.begin(), row.end(), 0);
    for (auto& row : mustnear_cons) fill(row.begin(), row.end(), 0);
    
    // 验证任务查找表状态
    for (auto& row : takeout) fill(row.begin(), row.end(), nullptr);
    for (auto& row : putin) fill(row.begin(), row.end(), nullptr);
    fill(close.begin(), close.end(), nullptr);
    fill(open.begin(), open.end(), nullptr);
    fill(pickup.begin(), pickup.end(), nullptr);
    fill(putdown.begin(), putdown.end(), nullptr);
    
    // 验证解析器状态
    if (nlp_parser) {
        parser::clear_static_state();
    }
    
    cout << "#(RDFW): Pre-test state verification completed" << endl;
    
    // print test name
    printf(GREEN "%s\n" RESET, GetTestName().c_str());  
    

    cout <<  "--------------------------------------------" << endl;
    cout << "Ready to Parse Env Infos." << endl;

    string env_str = GetEnvDes();

    // Try to Parse Env, write to Object, SmallObject
    if (ParseEnv(env_str) == false) {
        cout<<"ParseEnv Failed! Skipping this test."<<endl;
        cout<<"# Test skipped due to environment parsing failure"<<endl;
        return;  // Exit Plan() gracefully, allowing Fini() to be called
    } else {
        cout << endl << "ParseEnv finished" << endl;
    }

    cout <<  "--------------------------------------------" << endl;
    cout << "Ready to Parse Tasks and Cons." << endl;

    const string task_str = GetTaskDes();

    if(task_str[0]=='(')  {  // check IT or NT
        isNaturalParse=0;   
    } else {
        isNaturalParse=1;
    }

    // parse natrual language
    if (isNaturalParse) {
        ParseNaturalLanguage(task_str);
    } else {
        if (ParseInstruction(task_str) == false) {
            cout << "Parse Natural Language Failed. Skipping this test." << endl;
            cout << "# Test skipped due to instruction parsing failure" << endl;
            return;  // Exit Plan() gracefully, allowing Fini() to be called
        }
    }

    // Parse Info Complements
    for (const auto &v : infos){
        ParseInfo(v);
    }
    

    if(stage == 2){
        PrintEnv();
    // ==================== 约束纠错与补全 ====================
    // 在解析完成后、位置推断前进行约束纠错
    ApplyOpenCloseCorrection();
    ApplyMustInConstraintCorrection();
    ApplyMustNearConstraintCorrection();
    
    }
    // Find Human Infomation from Objects
    human = nullptr;
    for (const auto &obj : objects) {
        if (obj->sort == "human") {
            human = ObjectPtrCast<BigObject>(obj);
            break;
        }
    }

    // print environment, instructions, 
    PrintEnv();
    PrintInstruction();
    cout<<"Parse Tasks, Cons and Infos finished."<<endl;
    cout <<  "--------------------------------------------" << endl;



    /*=======================约束条件规划===================*/

    cout << "Ready to Cons  plan" << endl;
    Cons_plan();
    FilterConstraintsByTaskConflicts();
    cout<<"cons_plan finished"<<endl;
    cout <<  "--------------------------------------------" << endl;

    /*=======================任务优化===================*/
    tasks = TaskOptimization();
    cout<<"taskoptimization finished"<<endl;
    cout <<  "--------------------------------------------" << endl;

    /*=======================任务执行===================*/
    if (false)
    {
        cout << "2222222" << endl;
        cout << "2222222" << endl;
    }
    else
    {
        const size_t tasksSize = tasks.size();
        const bool performErrorCorrection = isErrorCorrection;//是否开启纠错模式
        cout << tasksSize << endl;

        // 检查并延迟多goto任务
        bool defer_multi_goto = CheckAndDeferMultiGoto();
        
        // 设置多goto模式标志
        //isMultiGotoMode = defer_multi_goto;

         // 执行主任务循环
         ExecuteMainTaskLoop(defer_multi_goto);

        /*============== mustchooseone and goto ==============*/
        // 只在“剩余启用的 goto < 2”时才调用 MustChooseOne，避免抢跑 goto
        if (solved_task_num == 0) {
            size_t remain_goto = 0;
            for (auto &t : tasks) if (t.isEnable && t.behave == "goto") ++remain_goto;
            if (remain_goto < 2) {
                MustChooseOne();
            } else {
                LOG(YELLOW "[Defer] skip MustChooseOne because multi-goto remain=%zu\n" RESET, remain_goto);
            }
        }

    }
    /*============== 检查阶段 ==============*/
    //检查一遍
    cout<<"-----check check check-----"<<endl;

    // 执行检查阶段
    ExecuteCheckPhase(CheckAndDeferMultiGoto());

    /*============== Multi-GOTO聚合 ==============*/
    // 执行Multi-GOTO聚合
    PrintEnv();
    ExecuteMultiGotoAggregation();

    /*============== 结果输出 ==============*/
    cout << endl
         << "Sovled Task Num:" << solved_task_num << "    " << "Expect Num:" << tasks.size() << endl;
}

/*============== 主任务循环 ==============*/
void RDFW::ExecuteMainTaskLoop(bool defer_multi_goto)
{
    /*if(stage==2)
    {
        SenseCurrentLocationOnly();
    }*/
    
    // ============== 统计puton任务中出现多次的大物体 ==============
    bigObjectTaskCount.clear();  // 清空之前的统计
    map<unsigned int, vector<unsigned int>> bigObjectTaskIndices;  // 大物体ID -> 任务索引列表
    
    for (size_t i = 0; i < tasks.size(); ++i) {
        if (tasks[i].isEnable && tasks[i].behave == "puton" && 
            !tasks[i].Y.empty() && tasks[i].Y[0] != nullptr) {
            unsigned int bigObjectId = tasks[i].Y[0]->id;
            bigObjectTaskCount[bigObjectId]++;
            bigObjectTaskIndices[bigObjectId].push_back(i);
        }
    }
    
    // 标记出现多次的大物体
    for (const auto& pair : bigObjectTaskCount) {
        if (pair.second >= 2) {
            unsigned int bigObjectId = pair.first;
            int taskCount = pair.second;
            LOG(GREEN "[MultiPuton] Big object %d (sort: %s) appears in %d puton tasks" RESET, 
                bigObjectId, objects[bigObjectId]->sort.c_str(), taskCount);
            
            // 标记这些任务为多puton任务
            for (unsigned int taskIdx : bigObjectTaskIndices[bigObjectId]) {
                tasks[taskIdx].isMultiPuton = true;  // 假设Instruction类有这个成员
                LOG(YELLOW "[MultiPuton] Marked task %d as multi-puton for object %d" RESET, 
                    taskIdx, bigObjectId);
            }
        }
    }
    // ============== 多puton统计结束 ==============
    
    const size_t tasksSize = tasks.size();
    for (task_index = 0; task_index < tasksSize; ++task_index)
    {
        // 跳过包含不存在物体的任务
        if (tasks[task_index].hasMissingObjects) {
            continue;
        }
        
        // 延后多 GOTO：本轮不做，留给末尾聚合
        if (defer_multi_goto && tasks[task_index].behave == "goto") {
            continue;
        }
        
        isPass = false;
         /*============== 风险预判 ================*/
        if(CalculateTaskRisk(tasks[task_index])>=2) {     //先行判断
            cout<<tasks[task_index].behave<<" "<<"的风险系数是："<<tasks[task_index].risk<<endl;
            stringstream ss;
            ss << tasks[task_index];
            LOG(GREEN "too many cons\n %s" RESET, ss.str().c_str());
            continue;
        }

        /*============== 任务执行 ================*/

        // —— 零动作预验证：仅 stage2 且当前判定为“已满足”的任务才触发
        bool zero_ok = ZeroActionPreCheck(tasks[task_index]);
        if (zero_ok) {
            stringstream ss; ss << tasks[task_index];
            LOG(GREEN "Task done (zero-action prevalidated)\n %s" RESET, ss.str().c_str());
            SetSolvedTaskNum(solved_task_num + 1);
            tasks[task_index].isEnable = false;
            AfterSolveTask(tasks[task_index]);
        } else if (SolveTask(tasks[task_index])) {
            stringstream ss; ss << tasks[task_index];
            LOG(GREEN "Task done\n %s" RESET, ss.str().c_str());
            SetSolvedTaskNum(solved_task_num + 1);
            tasks[task_index].isEnable = false;
            AfterSolveTask(tasks[task_index]);
        } else {
            stringstream ss; ss << tasks[task_index];
            LOG(GREEN "Task not done\n %s" RESET, ss.str().c_str());
            tasks[task_index].isEnable=0;
            tasks[task_index].isfalse=1;
        }
    }
}


/*============== 检查阶段 ==============*/
void RDFW::ExecuteCheckPhase(bool defer_multi_goto)
{
    for(task_index = 0; task_index < tasks.size(); ++task_index){
        // 跳过包含不存在物体的任务
        if (tasks[task_index].hasMissingObjects) {
            continue;
        }
        
        // 延后多 GOTO：检查一遍也不做，留给末尾聚合
        if (defer_multi_goto && tasks[task_index].isEnable && tasks[task_index].behave == "goto") {
            continue;
        }
        if(tasks[task_index].isEnable&&CalculateTaskRisk(tasks[task_index])<2){
            bool zero_ok = ZeroActionPreCheck(tasks[task_index]);
            if (zero_ok) {
                stringstream ss; ss << tasks[task_index];
                LOG(GREEN "Task done (zero-action prevalidated)\n %s" RESET, ss.str().c_str());
                SetSolvedTaskNum(solved_task_num + 1);
                tasks[task_index].isEnable = false;
                AfterSolveTask(tasks[task_index]);
            } else if (SolveTask(tasks[task_index])) {
                stringstream ss; ss << tasks[task_index];
                LOG(GREEN "Task done\n %s" RESET, ss.str().c_str());
                SetSolvedTaskNum(solved_task_num + 1);
                tasks[task_index].isEnable = false;
                AfterSolveTask(tasks[task_index]);
            }
        }
    }
}





/**====================== 约束条件规划 =========================== */


void RDFW::Cons_plan(){
    int x;
    for(auto cons:not_infoConstrains){   
        // 跳过包含不存在物体的约束
        if (cons.hasMissingObjects) {
            continue;
        }
        
        if(cons.behave=="on") {
            if(cons.X[0]->location!=cons.Y[0]->location) putdown_cons[cons.X[0]->id][cons.Y[0]->location]++;
            else if(cons.X[0]->id==plate_id||cons.X[0]->id==hold_id) putdown_cons[cons.X[0]->id][cons.Y[0]->location]++;
        }
        else if(cons.behave=="inside"||cons.behave=="in") {
             auto small=dynamic_pointer_cast<SmallObject>(cons.X[0]);
            if(small->inside!=cons.Y[0]->id) putin_cons[cons.X[0]->id][cons.Y[0]->id]++;
        }
        else if(cons.behave == "near"||cons.behave == "nextto") {
            if(cons.Y[0]->location!=cons.X[0]->location) //如果约束没有触犯
            {
            if(cons.Y[0]->location!=UNKNOWN) move_cons[cons.X[0]->id][cons.Y[0]->location]++;
            if(cons.X[0]->location!=UNKNOWN) move_cons[cons.Y[0]->id][cons.X[0]->location]++;
            }
        } 
        else if(cons.behave == "plate") toplate_cons[cons.X[0]->id]++; 
        else if(cons.behave == "opened") {
            auto cont=dynamic_pointer_cast<Container>(cons.X[0]);
            if(cont->isOpen!=1) open_cons[cons.X[0]->id]++;  
        }
        else if(cons.behave == "closed")
        {   
            auto cont=dynamic_pointer_cast<Container>(cons.X[0]);
            if(cont->isOpen==1) close_cons[cons.X[0]->id]++; 
        }   
    }
    for(auto cons:notnot_infoConstrains){  
        // 跳过包含不存在物体的约束
        if (cons.hasMissingObjects) {
            continue;
        }
        
            if(cons.behave=="on"&&cons.X[0]->location==cons.Y[0]->location) {
                auto small=dynamic_pointer_cast<SmallObject>(cons.X[0]);
               if(small->inside!=cons.Y[0]->id) cons.X[0]->is_keep++;
               mustnear_cons[cons.X[0]->id][cons.Y[0]->id]++;
            }
            else if(cons.behave=="near"&&cons.Y.size()>0&&cons.X[0]->location==cons.Y[0]->location){
                 cons.X[0]->is_keep++;
                  cons.Y[0]->is_keep++;
                  mustnear_cons[cons.X[0]->id][cons.Y[0]->id]++;
            }
            else if(cons.behave=="plate"&& plate_id==cons.X[0]->id)fromplate_cons[cons.X[0]->id]++;
            else if(cons.behave=="inside"||cons.behave=="in")
            {
            auto small=dynamic_pointer_cast<SmallObject>(cons.X[0]);
            if(small->inside==cons.Y[0]->id)  takeout_cons[cons.X[0]->id][cons.Y[0]->id]++;
            } 
           else if(cons.behave=="closed") {
             auto cont=dynamic_pointer_cast<Container>(cons.X[0]);
            if(cont->isOpen!=1) open_cons[cons.X[0]->id]++;
           }
           else if(cons.behave=="opened") {
            auto cont=dynamic_pointer_cast<Container>(cons.X[0]);
            if(cont->isOpen!=1) close_cons[cons.X[0]->id]++;
           }
    }
    for(auto cons:not_taskConstrains){
        // 跳过包含不存在物体的约束
        if (cons.hasMissingObjects) {
            continue;
        }
        
        //这里不用判断一开始是否触犯约束
         if(cons.behave=="takeout") takeout_cons[cons.X[0]->id][cons.Y[0]->id]++;
         else if(cons.behave=="putin") putin_cons[cons.X[0]->id][cons.Y[0]->id]++;
         else if(cons.behave=="puton") putdown_cons[cons.X[0]->id][cons.Y[0]->location]++;
         else if(cons.behave=="goto") goto_cons[cons.X[0]->location]++;
         else if(cons.behave=="open") open_cons[cons.X[0]->id]++;
         else if(cons.behave=="close") close_cons[cons.X[0]->id]++;
         else if(cons.behave=="pickup") pickup_cons[cons.X[0]->id]++;
         else if(cons.behave=="give") givehuman_cons[cons.X[0]->id]++;
         else if(cons.behave == "putdown") putdown1_cons[cons.X[0]->id]++ ;
    }
    if(hold_id>0) {
        x=hold_id;
     if(objects[x]->is_keep>putdown1_cons[x]+putdown_cons[x][location]+fromplate_cons[x]){
        cout<<"the hold object must putdown here!"<<endl;
         PutDown(x);
     }
     }
    if(plate_id>0) 
    {
        x=plate_id;
     if(objects[x]->is_keep>putdown1_cons[x]+putdown_cons[x][location]+fromplate_cons[x]){
        cout<<"the plate object must putdown here!"<<endl;
        if(hold_id>0) PutDown(hold_id);
        FromPlate(x);
         PutDown(x);
     }
    }
    
}


void RDFW::FilterConstraintsByTaskConflicts() {
    // 只计算goto_cons和open_cons与多少任务冲突，超过4个则舍弃这个约束
    // 定义一个需要舍弃的gotocons数组
    int discard_gotoconsloc[100] = {0};
    int discard_gotoconsloc_count[100] = {0};
    int sum_conflict_count=0;
    int max_effect=0;
    int max_effect_loc=0;
    int i=0;//计数
    // 1. 处理goto_cons
    for (int loc = 0; loc < (int)goto_cons.size(); ++loc) {
        if (goto_cons[loc] > 0) {
            int conflict_count = 0;
            for (const auto& task : tasks) {
                // 判断任务是否会与goto约束冲突
                if (task.isEnable) {
                    if ((task.behave == "goto" && task.X.size() > 0 && task.X[0]->location == loc) ||
                        (task.behave == "putin" && task.Y.size() > 0 && task.Y[0]->location == loc) ||
                        (task.behave == "putin" && task.X.size() > 0 && task.X[0]->location == loc)||
                        (task.behave == "puton" && task.Y.size() > 0 && task.Y[0]->location == loc)||
                        (task.behave == "puton" && task.X.size() > 0 && task.X[0]->location == loc)||
                        (task.behave == "open" && task.X.size() > 0 && task.X[0]->location == loc)||
                        (task.behave == "close" && task.X.size() > 0 && task.X[0]->location == loc)||
                        (task.behave == "pickup" && task.X.size() > 0 && task.X[0]->location == loc)||
                        (task.behave == "give" && task.X.size() > 0 && task.X[0]->location == loc)||
                        (task.behave == "give" && task.Y.size() > 0 && task.Y[0]->location == loc)||
                        (task.behave == "takeout" && task.Y.size() > 0 && task.Y[0]->location == loc)) {
                        conflict_count++;
                        cout << "[FilterConstraintsByTaskConflicts] Conflict found between goto_cons at location " << loc << " and task " << task.behave << " at location " << task.X[0]->location << endl;
                    }
                }
            }
            if (conflict_count - goto_cons[loc] > 2) {
                discard_gotoconsloc[i] = loc;
                discard_gotoconsloc_count[i] = conflict_count;
                sum_conflict_count+=conflict_count;
                if (conflict_count - goto_cons[loc] > max_effect) {
                    max_effect = conflict_count - goto_cons[loc];
                    max_effect_loc = loc;
                }
                cout << "[FilterConstraintsByTaskConflicts] Conflict found between goto_cons at location " << loc << " with conflict count " << conflict_count << endl;
                i++;
            }
        }
    }

    
    if(sum_conflict_count>30) {
        goto_cons[max_effect_loc] = 0;
        cout << "[FilterConstraintsByTaskConflicts] Discarded goto_cons at location " << max_effect_loc << " due to " << max_effect << " conflicts." << endl;
    }
    else{
        while(i>=0){
            goto_cons[discard_gotoconsloc[i]] = 0;
            cout << "[FilterConstraintsByTaskConflicts] Discarded goto_cons at location " << discard_gotoconsloc[i] << " due to " << discard_gotoconsloc_count[i] << " conflicts." << endl;
            i--;
        }
    }
    
    // 2. 处理open_cons，逻辑同goto_cons
    int discard_openconsid[100] = {0};
    int discard_openconsid_count[100] = {0};
    int sum_conflict_count_open=0;
    int max_effect_open=0;
    int max_effect_id=0;
    int j=0;
    for (int id = 0; id < (int)open_cons.size(); ++id) {
        if (open_cons[id] > 0) {
            int conflict_count = 0;
            for (const auto& task : tasks) {
                // 判断任务是否会与open约束冲突
                // 这里假设冲突定义为：任务涉及该id，且是open/putin/takeout/puton/pickup/give等
                if (task.isEnable) {
                    if ((task.behave == "open" && task.X.size() > 0 && task.X[0]->id == id) ||
                        (task.behave == "putin" && task.Y.size() > 0 && task.Y[0]->id == id) ||
                        (task.behave == "putin" && task.X.size() > 0 && task.X[0]->id == id)||
                        (task.behave == "takeout" && task.Y.size() > 0 && task.Y[0]->id == id)||
                        (task.behave == "puton" && task.Y.size() > 0 && task.Y[0]->id == id)||
                        (task.behave == "pickup" && task.X.size() > 0 && task.X[0]->id == id)||
                        (task.behave == "give" && task.X.size() > 0 && task.X[0]->id == id)
                    ) {
                        conflict_count++;
                    }
                }
            }
            if (conflict_count - open_cons[id] > 2) {
                discard_openconsid[j] = id;
                discard_openconsid_count[j] = conflict_count;
                sum_conflict_count_open += conflict_count;
                if (conflict_count > max_effect_open) {
                    max_effect_open = conflict_count;
                    max_effect_id = id;
                }
                cout << "[FilterConstraintsByTaskConflicts] Conflict found between open_cons for id " << id << " with conflict count " << conflict_count << endl;
                j++;
            }
        }
    }
    
    if(sum_conflict_count_open>30) {
        open_cons[max_effect_id] = 0;
        cout << "[FilterConstraintsByTaskConflicts] Discarded open_cons for id " << max_effect_id << " due to " << max_effect_open << " conflicts." << endl;
    }
    else{
        for(int k = 0; k < j; k++){
            open_cons[discard_openconsid[k]] = 0;
            cout << "[FilterConstraintsByTaskConflicts] Discarded open_cons for id " << discard_openconsid[k] << " due to " << discard_openconsid_count[k] << " conflicts." << endl;
        }
    }



}


/**====================== 任务优化 =========================== */

//任务优化函数
vector<Instruction> RDFW::TaskOptimization()
{
    auto taskEvaluate = [](const string &behave) -> int
    {
        if (behave == "putin" || behave == "puton" || behave == "give")
            return 0;
        else if (behave == "takeout" ||behave == "putdown")
            return 1;
        else if (behave == "open" || behave == "close")
            return 2;
        else if ( behave == "pickup")
            return 3;
        else if (behave == "goto")
            return 4;
        else
            return 5;
    };
    /*
    auto TaskEquel =[](Instruction task1,Instruction task2) ->bool{
          if(task1.behave==task2.behave&&task1.behave!="pickup"&&task1.behave!="goto")
                if(task1.conditionX.sort==task2.conditionX.sort)
                    return true;
        if(task1.behave==task2.behave){
                if(task1.behave=="pickup"||task1.behave=="goto")
                return true;
            }

          return false;
    }*/
    // Sort tasks based on behavior evaluation and container grouping for takeout tasks
    std::sort(tasks.begin(), tasks.end(), [&](const Instruction &a, const Instruction &b)
              { 
                  int priority_a = taskEvaluate(a.behave);
                  int priority_b = taskEvaluate(b.behave);
                  
                  // If different task types, sort by priority
                  if (priority_a != priority_b) {
                      return priority_a < priority_b;
                  }
                  
                  // If both are takeout tasks, group by container (Y[0]->id)
                  if (a.behave == "takeout" && b.behave == "takeout") {
                      if (!a.Y.empty() && !b.Y.empty()) {
                          return a.Y[0]->id < b.Y[0]->id;
                      }
                  }
                  
                  // For other cases, maintain original order (stable sort)
                  return false;
              });
              

    vector<Instruction> optimizedTasks;
    optimizedTasks.reserve(tasks.size());

    bool hasGoto = false;
    for(int i=0;i<tasks.size();i++){
        // 跳过包含不存在物体的任务
        if (tasks[i].hasMissingObjects) {
            continue;
        }
        
        if(tasks[i].behave=="putin"){
             putin[tasks[i].X[0]->id][tasks[i].Y[0]->id]=&tasks[i];
        }
        else if(tasks[i].behave=="takeout"){
             takeout[tasks[i].X[0]->id][tasks[i].Y[0]->id]=&tasks[i];
        }
        else if(tasks[i].behave=="open"){
             open[tasks[i].X[0]->id]=&tasks[i];
        }
        else if(tasks[i].behave=="close"){
             close[tasks[i].X[0]->id]=&tasks[i];
        }
        else if(tasks[i].behave=="pickup"){
             pickup[tasks[i].X[0]->id]=&tasks[i];
        }
        else if(tasks[i].behave=="putdown"){
             putdown[tasks[i].X[0]->id]=&tasks[i];
        }
        optimizedTasks.push_back(tasks[i]);
      }

return optimizedTasks;
}




/**====================== 任务执行 =========================== */

/*========1)风险预判 ===============*/

//计算任务风险
int RDFW::CalculateTaskRisk(Instruction &t){
    t.risk=0;
   if(t.behave=="takeout")
   {
     auto small=dynamic_pointer_cast<SmallObject>(t.X[0]);
    if(small->inside!=t.Y[0]->id) return 0;//如果任务满足
       EnsureLocationCapacity(t.Y[0]->location);
     t.risk+=takeout_cons[t.X[0]->id][t.Y[0]->id]+goto_cons[t.Y[0]->location];
     t.risk+=open_cons[t.Y[0]->id];
    }

     else if(t.behave=="putin") {
         auto small=dynamic_pointer_cast<SmallObject>(t.X[0]);
          if(small->inside==t.Y[0]->id) return 0;
         EnsureLocationCapacity(t.Y[0]->location);

            t.risk+=putin_cons[t.X[0]->id][t.Y[0]->id]+open_cons[t.Y[0]->id]+move_cons[t.X[0]->id][t.Y[0]->location];
            if(t.X[0]->location!=t.Y[0]->location) t.risk+=goto_cons[t.Y[0]->location];
            CalculateStepRisk(t);

      }
      else if(t.behave=="puton") {
          EnsureLocationCapacity(t.Y[0]->location);

          t.risk+= putdown_cons[t.X[0]->id][t.Y[0]->location]+move_cons[t.X[0]->id][t.Y[0]->location]+putdown1_cons[t.X[0]->id];
        if(t.X[0]->location!=t.Y[0]->location) t.risk+=goto_cons[t.Y[0]->location];
        CalculateStepRisk(t);
      }
      else if (t.behave == "goto") {
          int loc = t.X[0]->location;
          t.risk += goto_cons[loc];
           // 调试日志，明确 t.risk 的组成
          std::cout << "[DBG] goto risk@loc=" << loc
                    << " bool=" << goto_cons[loc]
                   << std::endl;
      }

      else if(t.behave=="open") t.risk+=open_cons[t.X[0]->id]+goto_cons[t.X[0]->location];
      else if(t.behave=="close") t.risk+=close_cons[t.X[0]->id]+goto_cons[t.X[0]->location];
      else if(t.behave=="pickup") {
         CalculateStepRisk(t);
      }
      else if(t.behave=="give") {
         CalculateStepRisk(t);
         t.risk+=givehuman_cons[t.X[0]->id]+move_cons[t.X[0]->id][human->location]+putdown1_cons[t.X[0]->id];
         if(t.X[0]->location!=human->location) t.risk+=goto_cons[human->location];
         auto small=dynamic_pointer_cast<SmallObject>(t.X[0]);
      }
      else if(t.behave == "putdown")t.risk+=putdown1_cons[t.X[0]->id];
    //  t.risk+=t.X[0]->is_keep;
    int keep_penalty = t.X[0]->is_keep;
    // 特例：仅对 pickup，且与 near/next-to 的“对端对象”仍在同一位置时，
    // 视为 pickup 不破坏 near/next-to —— 不计入这部分 keep 惩罚。
    if (t.behave == "pickup") {
        int near_keep = 0;
        for (const auto &cons : notnot_infoConstrains) {
            if (cons.behave == "near") { // 你的“next to”在解析里等同于 near
                const auto aId = t.X[0]->id;
                bool a_is_X = (cons.X.size() > 0 && cons.X[0]->id == aId);
                bool a_is_Y = (cons.Y.size() > 0 && cons.Y[0]->id == aId);
                if (!(a_is_X || a_is_Y)) continue;

                // 找到与 t.X[0] 成 near 关系的“另一端对象”
                shared_ptr<Object> other =
                    a_is_X ? (cons.Y.size() ? cons.Y[0] : nullptr)
                           : (cons.X.size() ? cons.X[0] : nullptr);

                // 如果二者当前确实在同一位置，则“原地 pickup”不会破坏 near
                if (other && other->location == t.X[0]->location && t.X[0]->location != UNKNOWN) {
                    near_keep++;
                }
            }
        }
        // 只剔除 near/next-to 导致的 keep 惩罚，其它类型的 keep 仍然有效
        if (near_keep > 0) {
            keep_penalty = std::max(0, keep_penalty - near_keep);
        }
    }
    t.risk += keep_penalty;
      return t.risk;
}


//计算步骤风险------计算获取物体的约束值
int RDFW:: CalculateStepRisk(Instruction &t){
    if(t.X[0]->location!=location) t.risk+=goto_cons[t.X[0]->location];
    auto small=dynamic_pointer_cast<SmallObject>(t.X[0]);
    if(small->inside!=UNKNOWN&&small->inside!=NONE)t.risk+=open_cons[small->inside]+takeout_cons[small->id][small->inside];
    else if(small->inside==NONE) t.risk+=pickup_cons[small->id];
    return 1;
}

// ==== helpers for capacity & existence ====
inline void RDFW::EnsureLocationCapacity(int loc) {
    if (loc < 0) return;
    
    // 扩展位置相关的动态数组
    if (loc >= (int)posCorrectFlag.size()) posCorrectFlag.resize(loc + 1, true); // 位置维度
    if (loc >= (int)posSensedFlag.size()) posSensedFlag.resize(loc + 1, false);
    if (loc >= (int)locationSensedObjects.size()) locationSensedObjects.resize(loc + 1);
    
    // 扩展约束数组
    if (loc >= (int)goto_cons.size()) goto_cons.resize(loc + 1, 0);
    if (loc >= (int)putdown1_cons.size()) putdown1_cons.resize(loc + 1, 0);
    if (loc >= (int)open_cons.size()) open_cons.resize(loc + 1, 0);
    if (loc >= (int)close_cons.size()) close_cons.resize(loc + 1, 0);
    if (loc >= (int)pickup_cons.size()) pickup_cons.resize(loc + 1, 0);
    if (loc >= (int)givehuman_cons.size()) givehuman_cons.resize(loc + 1, 0);
    if (loc >= (int)fromplate_cons.size()) fromplate_cons.resize(loc + 1, 0);
    if (loc >= (int)toplate_cons.size()) toplate_cons.resize(loc + 1, 0);
    if (loc >= (int)rightlocation.size()) rightlocation.resize(loc + 1, false);
    
    // 扩展二维约束数组
    if (loc >= (int)putin_cons.size()) putin_cons.resize(loc + 1, vector<int>(putin_cons.empty() ? 0 : putin_cons[0].size(), 0));
    if (loc >= (int)takeout_cons.size()) takeout_cons.resize(loc + 1, vector<int>(takeout_cons.empty() ? 0 : takeout_cons[0].size(), 0));
    if (loc >= (int)putdown_cons.size()) putdown_cons.resize(loc + 1, vector<int>(putdown_cons.empty() ? 0 : putdown_cons[0].size(), 0));
    if (loc >= (int)move_cons.size()) move_cons.resize(loc + 1, vector<int>(move_cons.empty() ? 0 : move_cons[0].size(), 0));
    if (loc >= (int)mustnear_cons.size()) mustnear_cons.resize(loc + 1, vector<int>(mustnear_cons.empty() ? 0 : mustnear_cons[0].size(), 0));
    
    // 扩展任务查找表
    if (loc >= (int)takeout.size()) takeout.resize(loc + 1, vector<Instruction*>(takeout.empty() ? 0 : takeout[0].size(), nullptr));
    if (loc >= (int)putin.size()) putin.resize(loc + 1, vector<Instruction*>(putin.empty() ? 0 : putin[0].size(), nullptr));
    if (loc >= (int)close.size()) close.resize(loc + 1, nullptr);
    if (loc >= (int)open.size()) open.resize(loc + 1, nullptr);
    if (loc >= (int)pickup.size()) pickup.resize(loc + 1, nullptr);
    if (loc >= (int)putdown.size()) putdown.resize(loc + 1, nullptr);
}

inline void RDFW::EnsureObjectExists(unsigned id, bool prefer_small) {
    if (id >= objects.size()) {
        size_t last = objects.size();
        objects.resize(id + 1);
        for (size_t i = last; i < objects.size(); ++i) {
            objects[i] = std::make_shared<Object>((unsigned)i);
        }
    }
    if (prefer_small) {
        if (!std::dynamic_pointer_cast<SmallObject>(objects[id])) {
            objects[id] = std::make_shared<SmallObject>(objects[id]); // 基于已有 Object 包装
            smallObjects.push_back(std::dynamic_pointer_cast<SmallObject>(objects[id]));
        }
    }
}


/*========2)任务顺序执行 ===============*/

/*======a)做任务（边执行边判断）==========*/


//执行任务----主要是看有没有任务对象，服务stage2
bool RDFW::SolveTask(const Instruction &task) // 改
{
    bool success = false;//是否成功执行

    if (task.behave == "puton" || task.behave == "putin" || task.behave == "takeout")
    {
        if (task.X.empty() || task.Y.empty())
        {
            LogInstructionError(task);
            return false;
        }
        for (const auto &a : task.X)
        {
            cout<<task.behave<<" "<<a->sort<<"的风险系数是："<<task.risk<<endl;
            success = DoBehavious(task.behave, a->id, task.Y[0]->id);
            if (!success)
                break;
        }
    }
    else if (task.X.empty())
    {
        LogInstructionError(task);
        success = false;
    }
    else
    {
        for (const auto &a : task.X)
        {
            cout<<task.behave<<" "<<a->sort<<"的风险系数是："<<task.risk<<endl;
            success = DoBehavious(task.behave,a->id);
            if (!success)
                break;
        }
    }
    return success;
}

//执行行为---一个对象
bool RDFW::DoBehavious(const string &behavious, unsigned int x)
{
    if (behavious == "move" || behavious == "Move" || behavious == "Goto" || behavious == "goto")
        return SolveTask_Goto(x);
    else if (behavious == "pickup" || behavious == "PickUp")
        return SolveTask_PickUp(x);
    else if (behavious == "close" || behavious == "Close")
    {
        return SolveTask_Close(x);
    }

    else if (behavious == "open" || behavious == "Open")
    {
        return SolveTask_Open(x);
    }
    else if (behavious == "putdown" || behavious == "PutDown")
        return SolveTask_PutDown(x);
    else if (behavious == "give" || behavious == "Give")
        return SolveTask_Give(x);
    else
    {
           LOG("error task error task error task");
           return false;
    }
}

//执行行为---两个对象
bool RDFW::DoBehavious(const string &behavious, unsigned int a, unsigned int b) //dobehavious函数
{
    if (behavious == "putin" || behavious == "PutIn")
    {
        return SolveTask_Putin(a,b);
    }

    else if (behavious == "takeout" || behavious == "TakeOut")
    {
       return SolveTask_TakeOut(a,b);
    }

    else if (behavious == "puton" || behavious == "PutOn")
    {
        return SolveTask_PutOn(a,b);
    }
    else
    {
        LOG("error task error task error task");
        return false;
    }
}

//拿起物体的逻辑
bool RDFW::HoldSmallObject(unsigned int a)
{
    int t = 0;
    auto target_small = ObjectPtrCast<SmallObject>(objects[a]);
    ///这是stage1的逻辑
     if(stage==1)
    {
      if (plate_id == a)
        {
        if (hold_id != NONE) PutDown(hold_id);
        return FromPlate(a);
        }
       else if(hold_id==a) return true;
    if (hold_id != NONE) PutDown(hold_id);
    if(location!=target_small->location) Move(target_small->location);
    if(target_small->inside==NONE) return PickUp(a);
    else if(target_small->inside!=UNKNOWN)//说明小物体在容器里面
    {
       auto target_cont = ObjectPtrCast<Container>(objects[target_small->inside]);
       if(!target_cont->isOpen) Open(target_cont->id);
       return  TakeOut(a,target_cont->id);
    }
    return false;
    }
    //这是stage2的逻辑
    if (hold_id != a)
    {
        if (hold_id != NONE) PutDown(hold_id); //如果拿着物体，先放下
        if (plate_id == a)
        {
        return FromPlate(a);
        }

        while (1)
        {
            t++;
            if (target_small->location != UNKNOWN)
            {
                if (location != target_small->location)
                    if(Move(target_small->location)!=1)
                    {
                        if (t >= 2)  return 0;
                        GetSmallObjectStatus(a);
                        if(!IsKeepingGoing(task_index)) return 0;
                        continue;
                    }else{
                        SenseCurrentLocationOnly();
                        if(!HasObjectAtLocation(target_small->location, a))continue;
                    }
            //Airong:Move后感知，位置可能重新标记为UNKNOWN
                if(target_small->location == UNKNOWN ){
                    if (t >= 2)  return 0;
                    GetSmallObjectStatus(a);
                    if(!IsKeepingGoing(task_index)) return 0;
                    continue;
                }


                 if (target_small->inside == NONE||target_small->inside == UNKNOWN) //这里我想了想，可能不会有UNKOWN的情况
                {
                    
                     if (target_small->location == location && PickUp(a)) return 1;

                     // 先保证容量（这是“语句”，必须放在 if 条件外执行）
                     EnsureLocationCapacity(location);

                     // 然后再按条件判断
                     if ( posSensedFlag[location]
                          && target_small->location == location
                          && HasContainerAtLocation(location)
                          && [&]{
                                 unsigned int cont_id = GetContainerAtLocation(location);
                                 if (cont_id > 0) {
                                     auto cont = ObjectPtrCast<Container>(objects[cont_id]);
                                     return (cont && cont->isOpen);
                                 }
                                 return false;
                             }() )
                     {
                         if (TakeOut(a, GetContainerAtLocation(location))) return 1;
                         if (t >= 2) return 0;
                     }
                     else {
                         if (plate_id == UNKNOWN && FromPlate(a)) return 1;
                         target_small->location = UNKNOWN;
                         if (t >= 2) return 0;
                         GetSmallObjectStatus(a);
                         if (!IsKeepingGoing(task_index)) return 0;
                         continue;
                     }


                }


                else
                {
                    int initial_cont_id=target_small->inside;
                    int fl=TakeOutLogic(a,target_small->inside);
                    if(fl==1) return true;
                    else
                    {

                        if(fl==3)  {if (t >= 2)  return 0;GetSmallObjectStatus(a);}
                        else if(fl==2) {if (t >= 2)  return 0;GetBigObjectStatus(target_small->inside);}
                        else if(fl==4)
                        {
                            if (t >= 2)  return 0;
                            GetSmallObjectStatus(a);
                            if(Isinside(a,initial_cont_id))//a就在一开始容器里面
                            {
                                    if(Open(initial_cont_id)) return TakeOut(a,initial_cont_id); //认为骗我是关的
                                    else  GetBigObjectStatus(initial_cont_id);//open失败，本身不可能是open的，直接问容器
                            }
                            else
                            {
                                //小物体不在容器里了
                            }
                        }
                        if(!IsKeepingGoing(task_index)) return 0;
                        continue;
                    }

                }
            }
            else{
                  if (t >= 2)  return 0;
                GetSmallObjectStatus(a);
                if(!IsKeepingGoing(task_index)) return 0;
                continue;
            }

        }
    }
    return 1;
}

//从容器拿出物体的逻辑
int RDFW::TakeOutLogic(unsigned int small,unsigned int cont){ //返回2表示需要问大物体，返回3表示需要问小物体,返回4为优化情况
    auto target_cont = ObjectPtrCast<Container>(objects[cont]);
    if (!target_cont) return 2;                 // 先判空
    //Airong:
    if(target_cont->location==UNKNOWN)return 2;

    if(!target_cont->isOpen)
    {
     if(Open(cont))
        if(!TakeOut(small,cont)){/*GetSmallObjectStatus(small);*/return 3;}
        else return 1;
    else
    {
      if(sense(cont))
      {
        if(!TakeOut(small,cont)){/*GetSmallObjectStatus(small);*/return 3;}
        else return 1;
      }
      else {/*GetBigObjectStatus(cont);*/return 2;};
    }
    }
    else //不需要打开容器
    {
         if(!TakeOut(small,cont))
         {
            return 4;
         }
         else return 1;
    }
}


/*===============
  solve task流程
  ==============*/

//pickup任务
bool RDFW::SolveTask_PickUp(unsigned int a)
{
    if(putdown[a]!=nullptr){
        if(hold_id==a||plate_id==a) return true;
        else {
            cout<<"there is putdown task,no need to do this!"<<endl;
            return false;
        }
    }
    if(hold_id==a ||plate_id==a) return true;
    else return HoldSmallObject(a);
}

bool RDFW::SolveTask_PutDown(unsigned int a)
{
    if(pickup[a]!=nullptr){
        if(hold_id!=a&&plate_id!=a) return true;
        else {
            cout<<"there is pickup task,no need to do this!"<<endl;
            return false;
        }
    }
    if(hold_id==a){
        if(putdown_cons[a][location]) Move(findrightlocation(a));//判断此位置是否有约束//随便去一个地方，这里写2
        return PutDown(a);
    }
    else if(plate_id==a){
        if(hold_id>0) {
            PutDown(hold_id);
            if(putdown_cons[a][location]) Move(findrightlocation(a));
        }
        FromPlate(a);
        if(putdown_cons[a][location]) Move(findrightlocation(a));
       return PutDown(a);
    }
    else return true;
}

//goto任务
bool RDFW::SolveTask_Goto(unsigned int a)
{
    if(stage==1)
    {
        if(location==objects[a]->location){
        return true;
    }
    else return Move(objects[a]->location);
    }

    //stage2的情况
    if(location==objects[a]->location) return true;
    bool is_small=0;
    if(objects[a]->location==UNKNOWN)
    {
        if(dynamic_pointer_cast<SmallObject>(objects[a]) != nullptr)
        {
               is_small=1;
               GetSmallObjectStatus(a);
               if(!IsKeepingGoing(task_index)) return 0;
        }
        else
        {
        GetBigObjectStatus(a);
        if(!IsKeepingGoing(task_index)) return 0;
        }
    }
    int t=0;
    while(1)
    {
        t++;
    if(location==objects[a]->location) return true;
    else if(!Move(objects[a]->location))
    {
          if(t>=2) return false;
          if(is_small) {GetSmallObjectStatus(a);if(!IsKeepingGoing(task_index)) return 0;}
          else {GetBigObjectStatus(a);if(!IsKeepingGoing(task_index)) return 0;}
    }
    else return true;
    }
    return false;
}

//open任务
bool RDFW::SolveTask_Open(unsigned int a)
{
    auto cnt=dynamic_pointer_cast<Container>(objects[a]);
    if(close[a]!=nullptr){
        if(cnt->isOpen) return true;
        else {
            cout<<"there is close task,no need to do this!"<<endl;
            return false;
        }
    }
    if(cnt->isOpen) return true;
    if(hold_id!=NONE) PutDown(hold_id);
    if(stage==1)
    {
        if (location != objects[a]->location) Move(objects[a]->location);
        return Open(a);
    }

    //这是stage2
    if(objects[a]->location==UNKNOWN)
    {
        GetBigObjectStatus(a);
        if(!IsKeepingGoing(task_index)) return 0;
    }
     int t;
    while(1)
    {
        t++;
    if (location != objects[a]->location)
        if(!Move(objects[a]->location))
        {
            if(t>=2) return false;
            GetBigObjectStatus(a);
             if(!IsKeepingGoing(task_index)) return 0;
        }
    if(!Open(a))
    {
       if(t>=2) return false;
            GetBigObjectStatus(a);
             if(!IsKeepingGoing(task_index)) return 0;
    }
    else return true;
    }
    return false;
}

//close任务
bool RDFW::SolveTask_Close(unsigned int a)
{
    auto cnt=dynamic_pointer_cast<Container>(objects[a]);
    if(open[a]!=nullptr)
    {
        if(!cnt->isOpen) return true;
        else {
            cout<<"there is open task,no need to do this!"<<endl;
            return false;
        }
    }
    if(!cnt->isOpen) return true;
    if(hold_id!=NONE) PutDown(hold_id);
    if(stage==1)
    {
    if (location != objects[a]->location) Move(objects[a]->location);
    return Close(a);
    }

   //这是stage2

   if(objects[a]->location==UNKNOWN)
    {
        GetBigObjectStatus(a);
        if(!IsKeepingGoing(task_index)) return 0;
    }
     int t;
    while(1)
    {
        t++;
    if (location != objects[a]->location)
        if(!Move(objects[a]->location))
        {
            if(t>=2) return false;
            GetBigObjectStatus(a);
             if(!IsKeepingGoing(task_index)) return 0;
        }
    if(!Close(a))
    {
       if(t>=2) return false;
            GetBigObjectStatus(a);
             if(!IsKeepingGoing(task_index)) return 0;
    }
    else return true;
    }
    return false;
}

//give任务
bool RDFW::SolveTask_Give(unsigned int a)
  {
        if (human != nullptr){
            if(human->location==UNKNOWN)
             {
        GetBigObjectStatus(a);
        if(!IsKeepingGoing(task_index)) return 0;
        }
            if(objects[a]->location==human->location && plate_id!=a && hold_id!=a) return true;
            else return SolveTask_PutOn(a, human->id);
            }
        else
            LOG_ERROR("There are not human in Scene");

    return false;
}


//putin任务
bool RDFW::SolveTask_Putin(unsigned int a, unsigned int b)
{
    if(takeout[a][b]!=nullptr)
    {
        if(!Isinside(a,b)==0) return true;
               else
               {
            cout<<"there is takeout task,no need to do this!"<<endl;
            return false;
               }
    }
    if(Isinside(a,b)) return true;
    auto target_cont = ObjectPtrCast<Container>(objects[b]);
    if(stage==1)
    {
        //优化了一下规划，如果目标物体和容器在一起，先打开再picku
        if(objects[a]->location==objects[b]->location)
        {
        if (location != target_cont->location) Move(target_cont->location);
        if (target_cont->isOpen != 1) Open(b);
        HoldSmallObject(a);
        return PutIn(a,b);
        }
        else
        {
        if(!HoldSmallObject(a)) return false;
        if (location != target_cont->location) Move(target_cont->location);
        if (target_cont->isOpen != 1)
        {
        PutDown(a);
        Open(b);
        PickUp(a);
        }
        return PutIn(a,b);
        }
       return false;
    }
    //stage2的情况
  //open如果false可能的情况有两种：1.容器本身就是开着的，他骗我没开 2.b容器就不在这个位置
  //putin a b 如果false的情况有两种： 1.容器是关着的，骗我是开着的，我没有打开 2.b容器就不在这个位置上
    if(objects[b]->location==UNKNOWN)
    {
        GetBigObjectStatus(b);
        if(!IsKeepingGoing(task_index)) return 0;
    }
    if(!HoldSmallObject(a)) return false;
    
    int open_flag=-1;
    int try_times=0;
    while(1)
    {
      try_times++;
    if (location != target_cont->location)
        if(!Move(target_cont->location))
        {
            if(try_times>=2) return false;
            GetBigObjectStatus(b);
            if(!IsKeepingGoing(task_index)) return false;
            continue;
        }

    int open_flag=0;
    if (!target_cont->isOpen)
    {
        PutDown(a);
        if(Open(b))
        {
             PickUp(a);
             return PutIn(a,b);
        }
        else
        {
            if(sense(b)) {PickUp(a); return PutIn(a,b);}
            else
            {
            if(try_times>=2) return false;
            PickUp(a);
            GetBigObjectStatus(b);
            if(!IsKeepingGoing(task_index)) return false;
            continue;
            }
        }

    }
    else
    {
    if(!PutIn(a, b))
    {
        PutDown(a);
       if(Open(b)){PickUp(a);return PutIn(a,b);}
       else
       {
        if(try_times>=2) return false;
        PickUp(a);
        GetBigObjectStatus(b);
        if(!IsKeepingGoing(task_index)) return false;
        continue;
        }
    //    if(sense(b))
    //    {
    //     PutDown(hold_id);
    //     Open(b);
    //     PickUp(a);
    //     return PutIn(a,b);
    //    }
    //    else
    //    {
    //      if(try_times>=2) return false;
    //      GetBigObjectStatus(b);
    //      if(!IsKeepingGoing(task_index)) return false;
    //      continue;
    //    }
    }
    else return true;
    }
    }
}


//takeout任务
bool RDFW::SolveTask_TakeOut(unsigned int a, unsigned int b)
{
    if(putin[a][b]!=nullptr){
         if(Isinside(a,b)==0) return true;
        else {
            cout<<"there is putin task,no need to do this!"<<endl;
            return false;
        }
    }
    auto small = ObjectPtrCast<SmallObject>(objects[a]);


    auto target_cont = ObjectPtrCast<Container>(objects[b]);
    if (small->inside != target_cont->id && small->inside!=UNKNOWN)
    {
         return true;
    }
    
   
    if(stage==1)
    {
        if (hold!= nullptr) PutDown(hold->id);
        if (location != target_cont->location)Move(target_cont->location);
        if (target_cont->isOpen != 1)Open(target_cont->id);
        return TakeOut(a, target_cont->id);
    }
   ///这是stage2的逻辑 //对于takeout任务，inside未知，location未知，先去容器尝试takeout；或者先询问小物体，再判断任务是否已经完成
    if(target_cont->location==UNKNOWN){GetBigObjectStatus(b);if(!IsKeepingGoing(task_index)) return false;}
    if (hold!= nullptr) PutDown(hold->id);
    int t = 0;
    while(1)
    {
        t++;
    if (location != target_cont->location)
        if(!Move(target_cont->location))
        {
        if (t >= 2)  return 0;
        GetBigObjectStatus(b);
        if(!IsKeepingGoing(task_index)) return false;
        continue;
        }
    int fl=TakeOutLogic(a,b);
    if(fl==1||fl==3) return true; //返回3表示小物体不在容器里
    else if(fl==2) {if (t >= 2)  return 0;GetBigObjectStatus(b);}
    else if(fl==4)
    {
    if (t >= 2)  return 0;
    GetSmallObjectStatus(a);
    if(Isinside(a,b))//a就在一开始容器里面
    {
     if(Open(b)) return TakeOut(a,b); //认为骗我是关的
     else  GetBigObjectStatus(b);//open失败，本身不可能是open的，直接问容器
    }
    else
    {
    return true; //小物体不在容器里了
    }
    }
    if(!IsKeepingGoing(task_index)) return false;continue;
    }
}


//puton任务
bool RDFW::SolveTask_PutOn(unsigned int a, unsigned int b)
{
    //10.27
    auto small=dynamic_pointer_cast<SmallObject>(objects[a]);
    if(small->inside==NONE&&small->location==objects[b]->location&&plate_id!=a&&hold_id!=a) return true;
    //
    if(objects[b]->location==UNKNOWN)
    {
        GetBigObjectStatus(b);
         if(!IsKeepingGoing(task_index)) return false;
    }
    if (!HoldSmallObject(a)) return false;
    int t=2;
    if(bigObjectTaskCount[b]>5)t=4;
 
    while(1)
    {
        t--;
        if (location != objects[b]->location)
        {
            if(!Move(objects[b]->location))
            {
            if (t <= 0)  return 0;
            GetBigObjectStatus(b);
            if(!IsKeepingGoing(task_index)) return false;
            continue;
            }
            else{
                if(tasks[task_index].isMultiPuton)
                {
                    SenseCurrentLocationOnly();
                    if(!HasObjectAtLocation(objects[b]->location, b))continue;
                }
            }
        }
        return PutDown(a);
    }
    return true;
}



/*============== b)mustchooseone ==============*/

//必须要做一个任务
void RDFW::MustChooseOne(void){
    if(stage==1){
        cout<<"stage=1"<<endl;
        int flag=0;
         for(int i=0;i<tasks.size();i++){
                   if(tasks[i].risk<tasks[flag].risk){
                    flag=i;
                   }
               }
               task_index=flag;
               cout<<"Must Choose one:"<<tasks[flag].behave<<endl;
               if (SolveTask(tasks[task_index]))
                {
                    stringstream ss;
                    ss << tasks[task_index];
                    LOG(GREEN "Task done\n %s" RESET, ss.str().c_str());
                    SetSolvedTaskNum(solved_task_num + 1);
                    tasks[task_index].isEnable = false;
                    AfterSolveTask(tasks[task_index]);
                }
    }
    if(stage==2){
        cout<<"stage=2"<<endl;
        // —— 新增：若剩余启用的 goto ≥ 2，则不要在这里做选择（留给 Final-GOTO）
        size_t remain_goto = 0;
        for (auto &t : tasks) if (t.isEnable && t.behave == "goto") ++remain_goto;
        if (remain_goto >= 2) {
            LOG(YELLOW "[Defer] MustChooseOne returns early due to multi-goto=%zu\n" RESET, remain_goto);
            return;
        }
          int t=0;
        while(solved_task_num==0){
        int flag=0;
    
        t++;
        for(int i=0;i<tasks.size();i++){
            // 跳过包含不存在物体的任务
            if (tasks[i].hasMissingObjects) {
                continue;
            }
    
                   if((tasks[i].ask_times==0?(double)tasks[i].risk/2:(tasks[i].is_cheat?1000:tasks[i].risk))
                        <(tasks[flag].ask_times==0?(double)tasks[flag].risk/2:(tasks[flag].is_cheat?1000:tasks[flag].risk))){
                    flag=i;
                   }
               }
                task_index=flag;
               cout<<"Must Choose one:"<<tasks[flag].behave<<endl;
               if(t>3){
                SolveTask(tasks[flag]);
                break;
               }
               if(tasks[flag].risk>=4){   //怀疑是否有陷阱
             if(tasks[flag].behave!="open"||tasks[flag].behave!="close") {
                 if (!tasks[flag].X.empty()) {
                     GetSmallObjectStatus(tasks[flag].X[0]->id);
                 }
             }
                if(IsKeepingGoing(flag)!=1) continue;
               }
               if(tasks[flag].isfalse) {
                if (!tasks[flag].X.empty()) {
                    GetSmallObjectStatus(tasks[flag].X[0]->id);
                }
                if(IsKeepingGoing(flag)!=1) continue;
               }

            // ===【新增】零动作预验证：先试；失败(含 fake)立刻回落到 SolveTask ===
            bool zero_ok = ZeroActionPreCheck(tasks[task_index]);
            if (zero_ok) {
                std::stringstream ss; ss << tasks[task_index];
                LOG(GREEN "Task done (zero-action prevalidated)\n %s" RESET, ss.str().c_str());
                SetSolvedTaskNum(solved_task_num + 1);
                tasks[task_index].isEnable = false;
                AfterSolveTask(tasks[task_index]);
                break;
            }

            if (SolveTask(tasks[task_index]))
                {
                    stringstream ss;
                    ss << tasks[task_index];
                    LOG(GREEN "Task done\n %s" RESET, ss.str().c_str());
                    SetSolvedTaskNum(solved_task_num + 1);
                    tasks[task_index].isEnable = false;
                    AfterSolveTask(tasks[task_index]);
                    break;
                }
        }
    }
}
    



/**============== 附：goto任务处理============== */

//检查是否有多goto任务
bool RDFW::CheckAndDeferMultiGoto()
{
    size_t goto_count_enabled = 0;
    for (auto &t : tasks) if (t.behave == "goto") ++goto_count_enabled;
    const bool defer_multi_goto = (goto_count_enabled >= 2);
    if (defer_multi_goto)
        LOG(YELLOW "[Defer] multi-goto detected: %zu tasks; skip in main loop, handle at final aggregation\n" RESET, goto_count_enabled);
    return defer_multi_goto;
}

//执行多goto任务
void RDFW::ExecuteMultiGotoAggregation()
{
    // =========================
    // [FINAL] Multi-GOTO 聚合（低风险 hub + 候选筛选 + 上限 10 + 最终停留）
    // =========================
    
    // 确保多goto模式标志已设置，跳过Sense操作
    isMultiGotoMode = true;
    LOG(GREEN "[MultiGoto] Executing multi-goto aggregation - skipping Sense operations\n" RESET);
    
    // 1) 收集启用中的 goto 任务对应的小物体
    // 分类收集 goto 任务中的大物体和小物体
    std::vector<unsigned> small_cand_ids;
    std::vector<unsigned> big_cand_ids;

    std::vector<unsigned> small_cand_ids_disable;
    small_cand_ids.reserve(32);
    big_cand_ids.reserve(32);
    for (auto &t : tasks) {
        if(t.behave == "goto"){
            if(t.isEnable){
                for (auto &x : t.X) {
                    if (std::dynamic_pointer_cast<SmallObject>(x))
                        small_cand_ids.push_back(x->id);
                    else if (std::dynamic_pointer_cast<BigObject>(x))
                        big_cand_ids.push_back(x->id);}
                }else{
                    for (auto &x : t.X) {
                        if (std::dynamic_pointer_cast<SmallObject>(x))
                            small_cand_ids_disable.push_back(x->id);
                    }
                }
            }
        }


    // 删掉small_cand_ids_disable中goto约束大于2的小物体
    small_cand_ids_disable.erase(
        std::remove_if(
            small_cand_ids_disable.begin(),
            small_cand_ids_disable.end(),
            [&](unsigned id) {
                int loc = (objects[id] ? objects[id]->location : UNKNOWN);
                int risk = (loc != UNKNOWN && loc >= 0) ? goto_cons[loc] : 99;
                return risk > 2;
            }
        ),
        small_cand_ids_disable.end()
    );
   

    /*不需要去重，不能有重复的任务---比赛规则
    // 去重
    std::sort(small_cand_ids.begin(), small_cand_ids.end());
    small_cand_ids.erase(std::unique(small_cand_ids.begin(), small_cand_ids.end()), small_cand_ids.end());
    std::sort(big_cand_ids.begin(), big_cand_ids.end());
    big_cand_ids.erase(std::unique(big_cand_ids.begin(), big_cand_ids.end()), big_cand_ids.end());
    // 计算每个小物体：去到其位置 + 拿起小物体 + 移动小物体违反的约束值（不考虑能否移动到 hub）
    // 如果小物体有 near 约束（即 near_cons[id] > 0），则视为不能移动，风险值设为极大
    */


    // 得到约束值小于2的小物体
    struct SmallGotoPickupInfo {
        unsigned id;
        int goto_risk = 0;      // 去到小物体位置的约束
        int pickup_risk = 0;    // 拿起小物体的约束
        int mustnear_risk = 0;  // mustnear 约束
        int total_risk = 0;     // 总约束
        bool can_move = true;   // 是否能移动
    };
    std::vector<SmallGotoPickupInfo> small_goto_infos;
    std::vector<unsigned> small_lowrisk_ids; // 约束值小于2的小物体id
    // 小物体：计算 goto/pickup/mustnear
    for (auto id : small_cand_ids) {
        SmallGotoPickupInfo info;
        info.id = id;
        // 2. 去到小物体当前位置
        int loc = (objects[id] ? objects[id]->location : UNKNOWN);
        info.goto_risk = (loc != UNKNOWN && loc >= 0) ? goto_cons[loc] : 99;
        // 3. 拿起小物体的约束
        info.pickup_risk = pickup_cons[id];
        // 4. mustnear 约束（统计所有位置的 mustnear_cons[id][*] 之和）
        int mustnear_sum = 0;
        for (int i = 0; i < (int)mustnear_cons[id].size(); ++i) mustnear_sum += mustnear_cons[id][i];
        info.mustnear_risk = mustnear_sum;
        // 5. 总约束
        info.total_risk = info.goto_risk + info.pickup_risk + info.mustnear_risk;
        info.can_move = (info.total_risk < 2);
        LOG(GREEN "[MultiGoto] Object[%u]: goto_risk=%d, pickup_risk=%d, mustnear_risk=%d, total_risk=%d, can_move=%s\n" RESET,
            id, info.goto_risk, info.pickup_risk, info.mustnear_risk, info.total_risk, info.can_move ? "true" : "false");
        if (info.total_risk < 3) {
            small_lowrisk_ids.push_back(id);
        }
        small_goto_infos.push_back(info);
    }
    // 大物体：仅统计 goto 约束值
    struct BigGotoInfo {
        unsigned id;
        int goto_risk = 0;
    };
    std::vector<BigGotoInfo> big_goto_infos;
    for (auto id : big_cand_ids) {
        BigGotoInfo info;
        info.id = id;
        int loc = (objects[id] ? objects[id]->location : UNKNOWN);
        info.goto_risk = (loc != UNKNOWN && loc >= 0) ? goto_cons[loc] : 99;
        LOG(GREEN "[MultiGoto] BigObject[%u]: goto_risk=%d\n" RESET, id, info.goto_risk);
        big_goto_infos.push_back(info);
    }
    
    
    LOG(GREEN "[MultiGoto] small_cand_id_disable: ");
    for(auto id : small_cand_ids_disable){
        LOG(GREEN "%u ", id);
    }
    LOG(GREEN "\n" RESET);
    LOG(GREEN "[MultiGoto] small_cand_ids: ");
    for(auto id : small_cand_ids){
        LOG(GREEN "%u ", id);
    }
    LOG(GREEN "\n" RESET);
    LOG(GREEN "[MultiGoto] big_cand_ids: ");
    for(auto id : big_cand_ids){
        LOG(GREEN "%u ", id);
    }
    LOG(GREEN "\n" RESET);
    LOG(GREEN "[MultiGoto] small_cand_ids.size()=%zu, big_cand_ids.size()=%zu\n" RESET, 
        small_cand_ids.size(), big_cand_ids.size());
    LOG(GREEN "[MultiGoto] small_lowrisk_ids.size()=%zu\n" RESET, small_lowrisk_ids.size());

    if(small_lowrisk_ids.size()>0 || big_cand_ids.size()>0){
        LOG(GREEN "[MultiGoto] Entering main aggregation logic\n" RESET);

    // 只收集有物体的位置，避免遍历所有空位置
    std::set<int> occupied_locations;
    
    // 收集所有小物体和大物体的位置
    for (auto id : small_lowrisk_ids) {
        if (id < objects.size() && objects[id]) {
            int loc = objects[id]->location;
            if (loc >= 0 && loc < (int)rightlocation.size()) {
                occupied_locations.insert(loc);
            }
        }
    }
    for (auto id : big_cand_ids) {
        if (id < objects.size() && objects[id]) {
            int loc = objects[id]->location;
            if (loc >= 0 && loc < (int)rightlocation.size()) {
                occupied_locations.insert(loc);
            }
        }
    }
    for (auto id : small_cand_ids_disable) {
        if (id < objects.size() && objects[id]) {
            int loc = objects[id]->location;
            if (loc >= 0 && loc < (int)rightlocation.size()) {
                occupied_locations.insert(loc);
            }
        }
    }
    
    // 转换为vector，只包含有物体的位置
    std::vector<int> valid_locations(occupied_locations.begin(), occupied_locations.end());
    
    LOG(GREEN "[MultiGoto] Performance optimization: Only checking %zu occupied locations instead of %zu total locations\n" RESET, 
        valid_locations.size(), rightlocation.size());

    // 记录每个位置的约束统计
    struct LocationRiskInfo {
        int loc;
        int goto_risk;
        std::vector<int> move_cons_risks;     // 对每个小物体
        std::vector<int> putdown_cons_risks;  // 对每个小物体
        int total_risk;
    };
    std::vector<LocationRiskInfo> location_risks;

    for (int loc : valid_locations) {
        LocationRiskInfo info;
        info.loc = loc;
        info.goto_risk = goto_cons[loc];
        info.move_cons_risks.reserve(small_lowrisk_ids.size());
        info.putdown_cons_risks.reserve(small_lowrisk_ids.size());
        int sum_risk = info.goto_risk;
        for (auto id : small_lowrisk_ids) {
            int move_risk = move_cons[id][loc];
            int putdown_risk = putdown_cons[id][loc];
            info.move_cons_risks.push_back(move_risk);
            info.putdown_cons_risks.push_back(putdown_risk);
            sum_risk += move_risk + putdown_risk;
        }
        info.total_risk = sum_risk;
        location_risks.push_back(info);
    }

    // 1. 选出约束值小于3且可达的位置
    std::vector<int> low_risk_locs;
    int min_risk = 1000000;
    for (const auto& info : location_risks) {
        // 检查位置是否可达（在rightlocation中且不是UNKNOWN）
        if (info.loc >= 0 && info.loc < (int)rightlocation.size() && rightlocation[info.loc]) {
            if (info.total_risk < 3) {
                low_risk_locs.push_back(info.loc);
            }
            if (info.total_risk < min_risk) {
                min_risk = info.total_risk;
            }
        }
    }
   
    // 统计选出来的小物体在每个位置的数量（只统计有物体的位置）
    std::map<int, int> location_smallobjloc_counts;
    // 合并两个vector
    std::vector<unsigned int> combined_ids = small_lowrisk_ids;
    combined_ids.insert(combined_ids.end(), small_cand_ids_disable.begin(), small_cand_ids_disable.end());
    
    for (auto id : combined_ids) {
        if (id < objects.size() && objects[id]) {
            int loc = objects[id]->location;
            if (loc >= 0 && loc < (int)rightlocation.size()) {
                location_smallobjloc_counts[loc]++;
            }
        }
    }
    
    int chosen_loc = -1;

    // 优先选择大物体中约束值最小且小于2的位置，若没有，选择约束值最小的位置
    int best_big_risk = 1000000;
    int best_big_loc = -1;

    // 直接使用小物体聚集数量大于2且数量最多的位置作为 chosen_loc

    int most_smallobj_loc = -1;
    int most_smallobj_count = 0;


    for (const auto& pair : location_smallobjloc_counts) {
        int loc = pair.first;
        int count = pair.second;
        if (count >= 2) {
            cout << "[MultiGoto] location " << loc << " has " << count << " small objects" << endl;
            // 找到对应的location_risks信息
            int total_risk = 99; // 默认高风险
            for (const auto& risk_info : location_risks) {
                if (risk_info.loc == loc) {
                    total_risk = risk_info.total_risk;
                    break;
                }
            }
            if (count > most_smallobj_count && (count - total_risk) > 2) {
                // 检查位置可达
                if (loc >= 0 && loc < (int)rightlocation.size()) {
                    most_smallobj_count = count;
                    most_smallobj_loc = loc;
                    cout << "[MultiGoto] most_smallobj_loc " << most_smallobj_loc << endl;
                }
            }
        }
    }


    if (most_smallobj_loc != -1) {
        chosen_loc = most_smallobj_loc;
        rightlocation[chosen_loc] = true;
        LOG(YELLOW "[MultiGoto] smallobj_loc is found: %d\n" RESET, most_smallobj_loc);
    }
    else{
        LOG(YELLOW "[MultiGoto] smallobj_loc is not found: %d\n" RESET, most_smallobj_loc);
    }


    // 1. 在 big_cand_ids 中找约束值最小且小于2的位置

    if(chosen_loc == -1){
    for (auto big_id : big_cand_ids) {
        if (objects[big_id]) {
            int loc = objects[big_id]->location;
            // 确保位置可达
            if (loc >= 0 && loc < (int)rightlocation.size() && rightlocation[loc]) {
                for (const auto& info : location_risks) {
                    if (info.loc == loc && info.total_risk < 2) {
                        if (info.total_risk < best_big_risk) {
                            best_big_risk = info.total_risk;
                            best_big_loc = loc;}
                            }
                        }
                    }
                }
            }
        }


    if (best_big_loc != -1 ) {
        chosen_loc = best_big_loc;
    } 
    else if(chosen_loc == -1){
        LOG(YELLOW "[MultiGoto] bigobj_loc is not found: %d\n" RESET, best_big_loc);
        // 没有满足条件的大物体位置，选择所有可达位置中约束值最小的位置
        int best_risk = 1000000;
        for (const auto& info : location_risks) {
            // 确保位置可达
            if (info.loc >= 0 && info.loc < (int)rightlocation.size() && rightlocation[info.loc]) {
                if (info.total_risk < best_risk) {
                    best_risk = info.total_risk;
                    chosen_loc = info.loc;
                }
            }
        }
        LOG(YELLOW "[MultiGoto] No optimal location found, using first available: %d\n" RESET, chosen_loc);
    }
    
    // 如果仍然没有选择到位置，选择第一个可达的位置
    if (chosen_loc == -1) {
        for (int loc = 0; loc < (int)rightlocation.size(); ++loc) {
            if (rightlocation[loc]) {
                chosen_loc = loc;
                LOG(YELLOW "[MultiGoto] No optimal location found, using first available: %d\n" RESET, loc);
                break;
            }
        }
    }
    

    // 验证选择的位置是否可达
    if (chosen_loc == -1 || !rightlocation[chosen_loc]) {
        LOG(RED "[MultiGoto] ERROR: No valid location found! chosen_loc=%d, rightlocation[%d]=%d\n" RESET, 
            chosen_loc, chosen_loc, chosen_loc >= 0 && chosen_loc < (int)rightlocation.size() ? rightlocation[chosen_loc] : -1);
        // 选择机器人当前位置作为备选
        chosen_loc = location;
        LOG(YELLOW "[MultiGoto] Using current location as fallback: %d\n" RESET, chosen_loc);
    }
    
    LOG(GREEN "Final-GOTO: choose hub=%d\n" RESET,chosen_loc);

        // 4) 两层过滤得到最终搬运集合 move_set
        std::vector<unsigned> move_set;
        move_set.reserve(small_lowrisk_ids.size());

        // 使用 small_lowrisk_ids 作为搬运集合
        for (auto id : small_lowrisk_ids) {
            // 重新定义lambda函数，因为作用域问题
            auto ViolationsIfGoto = [&](unsigned id) -> int {
                if (id >= objects.size() || !objects[id]) return 99;
                int loc = objects[id]->location;
                if (loc == UNKNOWN || loc < 0) return 99;
                return goto_cons[loc];
            };
            auto SafeForIdAt = [&](unsigned id, int h) -> bool {
                if (h < 0) return false;
                return putdown_cons[id][h] == 0 && move_cons[id][h] == 0;
            };
            if (ViolationsIfGoto(id) < 2 && SafeForIdAt(id, chosen_loc)) {
                move_set.push_back(id);
            }
        }

        // 没有可搬的，就至少停在 chosen_loc
        if (move_set.empty()) {
            if (location != chosen_loc) Move(chosen_loc);
        } else {
            auto in_set = [&](unsigned x){
                return std::find(move_set.begin(), move_set.end(), x) != move_set.end();
            };

            // 护栏：hold/plate 不在集合且跨区会触发 move 约束时，先就地处理
            if (hold_id > 0 && !in_set(hold_id) && move_cons[hold_id][chosen_loc]) {
                PutDown(hold_id);
            }
            if (plate_id > 0 && !in_set(plate_id) && move_cons[plate_id][chosen_loc]) {
                if (hold_id > 0) PutDown(hold_id);
                FromPlate(plate_id);
                PutDown(plate_id);
            }

            // 若 hold/plate 在集合里，优先处理
            auto promote_front = [&](unsigned x){
                auto it = std::find(move_set.begin(), move_set.end(), x);
                if (it != move_set.end()) std::rotate(move_set.begin(), it, it + 1);
            };
            if (hold_id  > 0) promote_front(hold_id);
            if (plate_id > 0) promote_front(plate_id);

            // 已在 chosen_loc 的优先，其余按 id 升序
            std::stable_sort(move_set.begin(), move_set.end(), [&](unsigned a, unsigned b){
                auto sa = std::dynamic_pointer_cast<SmallObject>(objects[a]);
                auto sb = std::dynamic_pointer_cast<SmallObject>(objects[b]);
                int da = (sa && sa->location == chosen_loc) ? 0 : 1;
                int db = (sb && sb->location == chosen_loc) ? 0 : 1;
                return (da != db) ? (da < db) : (a < b);
            });

            // 实际搬运 ≤ 10 件
            const int MULTI_GOTO_LIMIT = 500;
            int moved = 0;
            int try_times = 0;
            for (unsigned id : move_set) {
                if (moved >= MULTI_GOTO_LIMIT) break;
                // 直接调用 SolveTask_PutOn ，把小物体移动到 hub
                if (SolveTask_PutOn(id, chosen_loc)) {
                    ++moved;
                    LOG(GREEN "Final-GOTO: moved obj[%u] to hub=%d (%d/%d) (via SolveTask_PutOn)\n" RESET,
                        id, chosen_loc, moved, MULTI_GOTO_LIMIT);
                }
            }

            // 收尾：最终停在 chosen_loc
            if (location != chosen_loc) Move(chosen_loc);
            LOG(GREEN "Final-GOTO: aggregation done, stay at hub=%d\n" RESET, chosen_loc);

            // 关闭剩余 goto，避免后续检查阶段拉走
            for (auto &t : tasks) {
                if (t.isEnable && t.behave == "goto") t.isEnable = false;
            }
        }
    } else {
        LOG(YELLOW "[MultiGoto] Skipping aggregation: small_lowrisk_ids.size()=%zu, big_cand_ids.size()=%zu\n" RESET, 
            small_lowrisk_ids.size(), big_cand_ids.size());
    }
    // =========================
    // [FINAL] Multi-GOTO 聚合结束
    // =========================
    LOG(GREEN "[MultiGoto] ExecuteMultiGotoAggregation finished\n" RESET);
}



// === Zero-Action Precheck helpers (stage2 only) ===

// 判定当前知识下，该任务是否“无需执行器动作即可满足”
bool RDFW::IsZeroActionSatisfy(const Instruction& t) const
{
    if (stage != 2) return false;       // 仅 stage2 启用
    if (!t.isEnable || t.X.empty()) return false;

    const std::string& bh = t.behave;

    auto is_small = [&](const shared_ptr<Object>& o){
        return std::dynamic_pointer_cast<SmallObject>(o) != nullptr;
    };
    auto is_container = [&](const shared_ptr<Object>& o){
        return std::dynamic_pointer_cast<Container>(o) != nullptr;
    };

    // 安全获取 X[0] / Y[0]
    auto X0 = t.X[0];
    shared_ptr<Object> Y0 = (t.Y.empty() ? nullptr : t.Y[0]);

    // —— 根据不同动作的“已满足”判定规则（不引入移动/开关/拿放）
    if (bh == "open") {
        if (!Y0 && X0) Y0 = X0;
        auto c = std::dynamic_pointer_cast<Container>(Y0);
        return (c && c->isOpen);
    }
    if (bh == "close") {
        if (!Y0 && X0) Y0 = X0;
        auto c = std::dynamic_pointer_cast<Container>(Y0);
        return (c && !c->isOpen);
    }
    if (bh == "goto") {
        return (X0 && location == X0->location && X0->location != UNKNOWN);
    }
    if (bh == "pickup") {
        return (X0 && (hold_id == X0->id || plate_id == X0->id));
    }
    if (bh == "putdown") {
        if (!X0) return false;
        if (hold_id == X0->id || plate_id == X0->id) return false;  // 还拿着/在盘上 -> 未完成
        auto s = std::dynamic_pointer_cast<SmallObject>(X0);
        return (s && s->inside == NONE);  // 已在地面（不在容器/手/盘）
    }
    if (bh == "putin") {
        if (!X0 || !Y0) return false;
        auto s = std::dynamic_pointer_cast<SmallObject>(X0);
        auto c = std::dynamic_pointer_cast<Container>(Y0);
        return (s && c && s->inside == c->id); // 已在容器中
    }
    if (bh == "takeout") {
        if (!X0 || !Y0) return false;
        auto s = std::dynamic_pointer_cast<SmallObject>(X0);
        auto c = std::dynamic_pointer_cast<Container>(Y0);
        // “拿出”若已经不在该容器里（包含 UNKNOWN/NONE 或在其它容器），则视作已满足
        return (s && c && s->inside != c->id);
    }
    if (bh == "puton") {
        if (!X0 || !Y0) return false;
        auto s = std::dynamic_pointer_cast<SmallObject>(X0);
        if (!s) return false;
        if (hold_id == s->id || plate_id == s->id) return false; // 手/盘上，不算“已在台面”
        // puton 的判定：小物已在地面，且位置与 Y 相同
        return (s->inside == NONE && Y0->location != UNKNOWN && s->location == Y0->location);
    }

    return false;
}

// 执行“最多 1 次 Ask + 1 次 Sense”的轻量核验；通过则直接认为完成
bool RDFW::ZeroActionPreCheck(Instruction& t)
{
    if (stage != 2) return false;
    if (!IsZeroActionSatisfy(t)) return false;

    // —— 预算：最多 1 ask + 1 sense；不移动、不开关门、不拿放
    bool asked  = false;
    bool sensed = false;

    auto try_ask_small = [&](unsigned id){
        if (asked) return;
        GetSmallObjectStatus(id);
        t.ask_times++;      // 复用已有 ask 次数字段
        asked = true;
    };
    auto try_ask_big = [&](unsigned id){
        if (asked) return;
        GetBigObjectStatus(id);
        t.ask_times++;
        asked = true;
    };
    auto try_sense_obj = [&](unsigned id){
        if (sensed) return;
        if (id < objects.size() && objects[id] ) {
            if(objects[id]->location != location && goto_cons[objects[id]->location] == 0) {Move(objects[id]->location);}
            SenseCurrentLocationOnly();     // 只在当前格，避免引入 Move
            sensed = true;
        }
    };

    // —— 针对不同动作，挑选“最关键”的对象去问；若同格则附带一次感知
    // —— 针对不同动作，挑选“最关键”的对象去问；若同格则附带一次感知
    const std::string& bh = t.behave;
    auto X0 = t.X[0];
    shared_ptr<Object> Y0 = (t.Y.empty() ? nullptr : t.Y[0]);

    if (bh == "puton") {
        // ① 先核验 Y（大物体，容易被题面故意误报），用掉唯一一次 Ask
        if (Y0) { try_ask_big(Y0->id); try_sense_obj(Y0->id); }
        // ② 如仍有 Sense 名额，就地对 X 做一次感知（仅同格，不移动）
        if (X0) { try_sense_obj(X0->id); }

    } else if (bh == "open" || bh == "close") {
        if (!Y0 && X0) Y0 = X0;
        if (Y0) { try_ask_big(Y0->id); try_sense_obj(Y0->id); }

    } else if (bh == "putin" || bh == "takeout" || bh == "pickup" || bh == "putdown" || bh == "goto") {
        // 维持你当前策略：小物优先，其它按需
        if (X0 && std::dynamic_pointer_cast<SmallObject>(X0)) {
            try_ask_small(X0->id); try_sense_obj(X0->id);
        } else if (Y0 && std::dynamic_pointer_cast<SmallObject>(Y0)) {
            try_ask_small(Y0->id); try_sense_obj(Y0->id);
        } else if (X0) {
            try_ask_big(X0->id);   try_sense_obj(X0->id);
        } else if (Y0) {
            try_ask_big(Y0->id);   try_sense_obj(Y0->id);
        }
    }


    // —— 经过 0~1 次 ask + 0~1 次 sense 后再次确认
    if (IsZeroActionSatisfy(t)) {
        // 视为“已完成”；不调用执行器，只做与“成功执行”一致的收尾
        stringstream ss; ss << t;
        LOG(GREEN "[Zero-Action] validated as done\n %s" RESET, ss.str().c_str());

        return true;
    }
    { std::stringstream ss; ss << t;
        LOG(YELLOW "[LOG]: [FAKE-DONE] 这是 fake done task\n %s" RESET, ss.str().c_str()); }
    return false;

}
// === Zero-Action Precheck helpers (stage2 only) ===




/**============== b)状态判断函数============== */
//小物体、大物体、询问、Sense、inside判断、IsKeepingGoing、IsObjectSatisfy、IsInstructionInvoke、SearchConditionObject


//获取小物体状态---无返回值，直接更新状态
void RDFW::GetSmallObjectStatus(unsigned int a)
{
    tasks[task_index].ask_times++;
    if (isPass)
        return;

    string sureRet;
    if (isErrorCorrection && isAskTwice)
    {
        vector<string> ret;
        // 纠错模式下反复问，直到问出两次相同结果,认为正确。
        auto checkDouble = [&](const string &str) -> bool
        {
            for (const auto &v : ret)
                if (v == str)
                {
                    sureRet = str;
                    return true;
                }
            ret.push_back(str);
            return false;
        };
        while (checkDouble(AskLoc(a)) == false)
        {
        }
    }
    else
    {
        sureRet = AskLoc(a);
    }

    if (sureRet == "")
    {
        isPass = true;
        return;
    }
    vector<string> split;
    regex reg("[a-z 0-9]+");
    cmatch m;
    const char *pos = sureRet.data();
    const char *end = sureRet.data() + sureRet.size();
    for (; regex_search(pos, end, m, reg); pos = m.suffix().first)
    {
        split.push_back(m.str());
    }

    auto small = ObjectPtrCast<SmallObject>(objects[stoi(split[1])]);

    if (split[0] == "inside")
    {
        auto cont = dynamic_pointer_cast<Container>(objects[stoi(split[2])]);
        if (cont == nullptr){
            cout<<"The object is not a container!"<<endl;
            return GetSmallObjectStatus(a);
        }

       //small->location = cont->location;
        //10.23日改

      if(cont->location==UNKNOWN) GetBigObjectStatus(cont->id);
      small->location = cont->location;

        //10.23日改
        small->inside = cont->id;
        cont->smallObjectsInside.push_back(small);
    }

    else if (split[0] == "at")
    {
        small->location = stoi(split[2]);
        if(small->inside>0){  //如果回答at，认为在地上，如果之前认为在容器里面，就要清除
             auto cont = dynamic_pointer_cast<Container>(objects[small->inside]);
             cont->DeleteObjectInside(small);
        }
        small->inside = NONE;

    }
    if (isErrorCorrection)
        EnsureLocationCapacity(small->location);

        posCorrectFlag[small->location] = false;
}

//获取大物体状态---无返回值，直接更新状态
void RDFW::GetBigObjectStatus(unsigned int a)
{
    tasks[task_index].ask_times++;
    if (isPass)
        return;
    string sureRet;
    sureRet = AskLoc(a);
    if (sureRet == "")
    {
        isPass = true;
        return;
    }
    vector<string> split;
    regex reg("[a-z 0-9]+");
    cmatch m;
    const char *pos = sureRet.data();
    const char *end = sureRet.data() + sureRet.size();
    for (; regex_search(pos, end, m, reg); pos = m.suffix().first)
    {
        split.push_back(m.str());
    }


    if (split[0] == "at")
    {
        objects[a]->location = stoi(split[2]);
    if(dynamic_pointer_cast<Container>(objects[a])!=nullptr)
    {
        auto cont=dynamic_pointer_cast<Container>(objects[a]);
          for (int i = 0; i < cont->smallObjectsInside.size(); i++)
            {
                cont->smallObjectsInside[i]->location=cont->location;
            }
    }
    }
    else {
        cout<<"The Big Object cant in container!!!!"<<endl;
        return GetBigObjectStatus(a);
    }
    // if (isErrorCorrection)
    //     posCorrectFlag[small->location] = false;
}

//询问
std::string RDFW::AskLoc(unsigned int a)
{
    if (isPass)
        return "";
    string str;
    do
    {
        str = Plug::AskLoc(a);
        LOG("AskLoc(%d)", a);
        if (str == "")
        {
            LOG_ERROR("AskLoc return empty string,may object (%d,%s) not exsit!", a, objects[a]->sort.c_str());
        }
    } while (str == "not_known");
    return str;
}

//感知位置---只能判断是否有这个物体，容器中的物体感知不到
void RDFW::Sense()
{
    if(stage==1) return;//stage1不需要感知

    if (isPass)
        return;

    
    // 改进：每次移动后都进行感知，更新物体位置和状态信息
    // 添加位置感知记录，避免重复感知
    LOG(GREEN "[Sense] Performing sensing at location %d\n" RESET, location);

    vector<unsigned int> A_, B_;
    shared_ptr<Container> container = nullptr;
    auto checkSmallObejct = [&A_](unsigned int id) -> bool
    {
        for (auto a : A_)
            if (a == id)
                return true;
        return false;
    };
    if (dynamic_pointer_cast<Container>(objects[location]) != nullptr)
    {
        container = dynamic_pointer_cast<Container>(objects[location]);
        Open(location);
    }
    Plug::Sense(A_);
    LOG("Sense");
    // 检查当前位置物品正确性
    for (auto s : smallObjects)
    {
        if (s->location == location && checkSmallObejct(s->id) == false)
        {
            s->location = UNKNOWN;
            if (container != nullptr && container->id == s->inside)
                container->DeleteObjectInside(s);
            s->inside = UNKNOWN;
        }
    }
    // 更新当前位置物品，利用容器合上关闭时返回值不同，确定物品状态。
    for (auto a : A_)
    {
        auto small = dynamic_pointer_cast<SmallObject>(objects[a]);
        if (small == nullptr || small == hold || small == plate)
            continue;
        if (small->location != location)
        {
            if (small->inside > 0)
            {
                ObjectPtrCast<Container>(objects[small->inside])->DeleteObjectInside(small);
            }
            small->location = location;
        }
        if (container)
        {
            small->inside = UNKNOWN;
            B_.push_back(a);
        }
        else
            small->inside = NONE;
    }
    if (container != nullptr && B_.size() > 0) {
        container->smallObjectsInside.clear();
        Close(location);
        vector<unsigned int> C_;
        Plug::Sense(C_);
        LOG("Sense");
        for (auto b : B_)
        {
            auto small = dynamic_pointer_cast<SmallObject>(objects[b]);
            for (auto c : C_)
            {
                if (b == c)
                {
                    small->inside = NONE;
                }
            }
            if (small->inside == UNKNOWN)
            {
                if (small->inside != location)
                {
                    small->inside = location;
                    container->smallObjectsInside.push_back(small);
                }
            }
        }
    }
    

    // 更新PosCorrectFlag
}


// 只感知当前位置的物体，并更新其感知位置
void RDFW::SenseCurrentLocationOnly()
{
    unsigned int sensed_container_id = NONE;
    // 获取当前位置
    int curr_loc = location;
    if (curr_loc < 0) return;

    // 检查位置是否已经感知过，避免重复感知
    if (curr_loc >= posSensedFlag.size()) {
        posSensedFlag.resize(curr_loc + 1, false);
        locationSensedObjects.resize(curr_loc + 1);  // 同时扩展物体记录数组
    }
    
    if (posSensedFlag[curr_loc]) {
        LOG(YELLOW "[SenseCurrentLocationOnly] Location %d already sensed, skipping to avoid redundancy\n" RESET, curr_loc);
        return;
    }

    // 感知当前位置的物体
    vector<unsigned int> sensed_ids;
    Plug::Sense(sensed_ids);

    // 标记当前位置已感知
    if (curr_loc >= posSensedFlag.size()) {
        posSensedFlag.resize(curr_loc + 1, false);
        locationSensedObjects.resize(curr_loc + 1);  // 同时扩展物体记录数组
    }
    posSensedFlag[curr_loc] = true;
    
    // 清空当前位置的感知记录，准备记录新的感知结果
    locationSensedObjects[curr_loc].object_ids.clear();
    locationSensedObjects[curr_loc].container_id = 0;
    locationSensedObjects[curr_loc].has_container = false;
    
    LOG(GREEN "[SenseCurrentLocationOnly] Location %d marked as sensed\n" RESET, curr_loc);

    // 遍历感知到的物体，更新其位置
    for (auto id : sensed_ids) {
        if (id > 0 && id < objects.size() && objects[id] != nullptr) {
            objects[id]->location = curr_loc;
            
            // 记录感知到的物体ID
            locationSensedObjects[curr_loc].object_ids.push_back(id);
            
            auto cont = std::dynamic_pointer_cast<Container>(objects[id]);
            if (cont) {
                // 记录容器ID和标记有容器（一个位置只能有一个大物体）
                locationSensedObjects[curr_loc].container_id = id;
                locationSensedObjects[curr_loc].has_container = true;
                sensed_container_id = id;
                LOG("[SenseCurrentLocationOnly] Container %d detected at location %d", id, curr_loc);
            } else {
                LOG("[SenseCurrentLocationOnly] Object %d detected at location %d", id, curr_loc);
            }
        }
    }
    
    // 检查原本应该在这个位置但没被感知到的物体
    // 1. 标记已感知到的物体（包括容器内的物体）
    std::vector<bool> sensed(objects.size(), false);
    for (auto id2 : locationSensedObjects[curr_loc].object_ids) {
        if (id2 > 0 && id2 < objects.size() && objects[id2] != nullptr) {
            sensed[id2] = true;
        }
    }
    
    // 2.检查这个容器是否开着
    bool container_is_open = false;
    if (sensed_container_id > 0 && sensed_container_id < objects.size() && objects[sensed_container_id] != nullptr) {
        auto cont = std::dynamic_pointer_cast<Container>(objects[sensed_container_id]);
        if (cont) {
            container_is_open = cont->isOpen;
        }
    }

    // 直接按照位置找，找到标记为在这个位置的所谓物体id
    std::vector<unsigned int> ids_at_curr_loc;
    for (unsigned int i = 1; i < objects.size(); ++i) {
        if (!objects[i]) continue;
        if (objects[i]->location == curr_loc) {
            ids_at_curr_loc.push_back(i);
        }
    }

    if(sensed_container_id == NONE || container_is_open == true){
        for (auto id : ids_at_curr_loc) {
            if (!sensed[id]) {
                objects[id]->location = UNKNOWN;
                LOG(YELLOW "[SenseCurrentLocationOnly] Object id=%u expected at %d but not sensed, set location UNKNOWN\n" RESET, id, curr_loc);
            }
        }
    }
    else{
        for (auto id : ids_at_curr_loc) {
            bool in_container = false;
            // 检查是否是小物体且在容器内
            auto small = std::dynamic_pointer_cast<SmallObject>(objects[id]);
            if (small && small->inside == sensed_container_id) {
                in_container = true;
            }
            
            if (!sensed[id] && !in_container) {
                objects[id]->location = UNKNOWN;
                LOG(YELLOW "[SenseCurrentLocationOnly] Object id=%u expected at %d but not sensed, set location UNKNOWN\n" RESET, id, curr_loc);
            }
        }
    }
}
// 位置感知物体记录访问函数实现
const RDFW::LocationSensedInfo& RDFW::GetLocationSensedInfo(int location) const {
    static RDFW::LocationSensedInfo empty_info;  // 返回空结构体作为默认值
    if (location >= 0 && location < locationSensedObjects.size()) {
        return locationSensedObjects[location];
    }
    return empty_info;
}

bool RDFW::HasObjectAtLocation(int location, unsigned int object_id) const {
    if (location >= 0 && location < locationSensedObjects.size()) {
        const auto& info = locationSensedObjects[location];
        for (auto id : info.object_ids) {
            if (id == object_id) return true;
        }
    }
    return false;
}

bool RDFW::HasContainerAtLocation(int location) const {
    if (location >= 0 && location < locationSensedObjects.size()) {
        return locationSensedObjects[location].has_container;
    }
    return false;
}

vector<unsigned int> RDFW::GetObjectsAtLocation(int location) const {
    if (location >= 0 && location < locationSensedObjects.size()) {
        return locationSensedObjects[location].object_ids;
    }
    return vector<unsigned int>();
}

unsigned int RDFW::GetContainerAtLocation(int location) const {
    if (location >= 0 && location < locationSensedObjects.size()) {
        return locationSensedObjects[location].container_id;
    }
    return 0;
}


//感知大物体是否在当前位置
bool RDFW::sense(unsigned int t) //返回值表示t对应的大物体是否在当前位置
{

    vector<unsigned int> A_;

    Plug::Sense(A_);
    LOG("Sense");
    int flagg = 0;
    // 用“感知到的对象 id”安全地更新
    for (auto id : A_) {
        if (t == id) flagg = 1;
        if (id < objects.size() && objects[id]) {
            if (objects[id]->location != location) {
                objects[id]->location = location;
                if (auto cont = std::dynamic_pointer_cast<Container>(objects[id])) {
                    for (auto &sp : cont->smallObjectsInside) {
                        if (sp) sp->location = location;
                    }
                }
                // 小物体分支无需强制改 inside，这里保持原有逻辑不动
            }
        }
    }
    return flagg;


}

//查找合适的位置
int RDFW::findrightlocation(unsigned int a)
{
    for(int i=0;i<(int)rightlocation.size();i++){
        if(rightlocation[i]&&move_cons[a][i]) return i;
    }
	return 2;
}

//判断小物体是否在容器里面
bool RDFW::Isinside(unsigned int a, unsigned int b){
    auto small = ObjectPtrCast<SmallObject>(objects[a]);
    if(small->inside==objects[b]->id) return true;
    else return false;
}


//判断任务是否继续---跟违反的约束有关
bool RDFW::IsKeepingGoing(unsigned int index){
    auto &t=tasks[index];
    t.risk=0;
      if(t.behave=="takeout")
      {
        auto small=dynamic_pointer_cast<SmallObject>(t.X[0]);
        if(small->inside==t.Y[0]->id){ //如果任务没有满足
            EnsureLocationCapacity(t.Y[0]->location);
        t.risk+=takeout_cons[t.X[0]->id][t.Y[0]->id]+goto_cons[t.Y[0]->location];
        t.risk+=open_cons[t.Y[0]->id];
            }
            }

        else if(t.behave=="putin") {
            auto small=dynamic_pointer_cast<SmallObject>(t.X[0]);
             if(small->inside!=t.Y[0]->id) //如果任务没有满足
             {
                EnsureLocationCapacity(t.Y[0]->location);
               t.risk+=putin_cons[t.X[0]->id][t.Y[0]->id]+open_cons[t.Y[0]->id]+move_cons[t.X[0]->id][t.Y[0]->location];
               if(t.X[0]->location!=t.Y[0]->location) t.risk+=goto_cons[t.Y[0]->location];
               CalculateStepRisk(t);
              }
         }
         else if(t.behave=="puton") {
             EnsureLocationCapacity(t.Y[0]->location);
             t.risk+= putdown_cons[t.X[0]->id][t.Y[0]->location]+move_cons[t.X[0]->id][t.Y[0]->location]+putdown1_cons[t.X[0]->id];
           if(t.X[0]->location!=t.Y[0]->location) t.risk+=goto_cons[t.Y[0]->location];
           CalculateStepRisk(t);
         }
         else if(t.behave=="goto") t.risk+=goto_cons[t.X[0]->location];
         else if(t.behave=="open") t.risk+=open_cons[t.X[0]->id]+goto_cons[t.X[0]->location];
         else if(t.behave=="close") t.risk+=close_cons[t.X[0]->id]+goto_cons[t.X[0]->location];
         else if(t.behave=="pickup") {
            CalculateStepRisk(t);
         }
         else if(t.behave=="give") {
            CalculateStepRisk(t);
            t.risk+=givehuman_cons[t.X[0]->id]+move_cons[t.X[0]->id][human->location]+putdown1_cons[t.X[0]->id];
            if(t.X[0]->location!=human->location) t.risk+=goto_cons[human->location];
            auto small=dynamic_pointer_cast<SmallObject>(t.X[0]);
         }
         else if(t.behave == "putdown")t.risk+=putdown1_cons[t.X[0]->id];
         t.risk+=t.X[0]->is_keep;
         if(t.risk>=2)
         {
            cout<<"cheating task!!"<<endl;
            cout<<"real cons num is "<<t.risk<<endl;
            t.is_cheat=0;
            return false;
         }
         else return true;
}

//判断物体是否满足条件
bool Condition::IsObjectSatisfy(const shared_ptr<Object> &target) const
 {
     if (!target || sort == "")
         return false;
     if (sort != target->sort)
         return false;
     if (color != "") // target maybe point to SmallObject
     {
         auto tem = dynamic_pointer_cast<SmallObject>(target); // Convert type safely
         if (tem && tem->color != color)
             return false;
     }
     return true;
 }
 
 //判断任务是否可执行
 bool Instruction::IsInstructionInvoke(const string &behave, const shared_ptr<Object> &x, const shared_ptr<Object> &y)
 {
     if (isEnable && behave == this->behave && conditionX.IsObjectSatisfy(x) && (y == nullptr || conditionY.IsObjectSatisfy(y)))
         return true;
     return false;
 }

//搜索条件物体
 void Instruction::SearchConditionObject(const shared_ptr<RDFW> &rdfw, bool is_every)
 {
     for (auto v : rdfw->objects)
     {
         if (conditionX.IsObjectSatisfy(v))
         {
             if (is_every == true || X.size() == 0)
                 X.push_back(v);
         }
         if (isUseY && conditionY.IsObjectSatisfy(v))
         {
             if (is_every == true || Y.size() == 0)
                 Y.push_back(v);
         }
     }
     
     // 验证是否找到了必要的物体
     if (X.empty()) {
         LOG_ERROR("Instruction Error: No object found matching conditionX (sort=%s, color=%s)", 
                   conditionX.sort.c_str(), conditionX.color.c_str());
         hasMissingObjects = true;  // 标记包含不存在的物体
     }
     if (isUseY && Y.empty()) {
         LOG_ERROR("Instruction Error: No object found matching conditionY (sort=%s, color=%s)", 
                   conditionY.sort.c_str(), conditionY.color.c_str());
         hasMissingObjects = true;  // 标记包含不存在的物体
     }
 }
 

/**====================== 原子动作 =========================== */
bool RDFW::TakeOut(unsigned int a, unsigned int b)
{
    auto small = ObjectPtrCast<SmallObject>(objects[a]);
    auto cont = ObjectPtrCast<Container>(objects[b]);
    LOG("TakeOut(%d,%s)(%d,%s)", a, small->sort.c_str(), b, cont->sort.c_str());
    if (Plug::TakeOut(a, b))
    {
        small->inside = NONE;
        cont->DeleteObjectInside(small);
        cont->isOpen=1;
        SetHold(small);
        UpdateTaskList("takeout", objects[a], objects[b]);
        takeout_cons[a][b]=0;
        return 1;
    }
    return 0;
}

bool RDFW::PutIn(unsigned int a, unsigned int b)
{
    auto cont = ObjectPtrCast<Container>(objects[b]);
    auto small = ObjectPtrCast<SmallObject>(objects[a]);
    if (!cont || !small) return 0; // 直接早退，避免 LOG 解引用空指针
    LOG("PutIn(%d,%s)(%d,%s)", a,small->sort.c_str() , b, cont->sort.c_str());
    if (Plug::PutIn(a, b))
    {

        small->inside = b;
        SetHold(nullptr);
        cont->smallObjectsInside.push_back(small);
        cont->isOpen=1;
        UpdateTaskList("putin", objects[a], objects[b]);
        putin_cons[a][b]=0;
        open_cons[b]=0;
        return 1;
    }
    return 0;
}
bool RDFW::Close(unsigned int a)
{
    shared_ptr<Container> container = ObjectPtrCast<Container>(objects[a]);
    if (!container) return 0;
    LOG("(%d,%s) has closed", a, container->sort.c_str());
    if (Plug::Close(a))
    {
        container->isOpen = false;
        UpdateTaskList("close", objects[a]);
        objects[a]->is_keep=0;
        close_cons[a]=0;
        return 1;
    }
    return 0;
}
bool RDFW::Open(unsigned int a)
{
    shared_ptr<Container> container = ObjectPtrCast<Container>(objects[a]);
    if (!container) return 0;
    LOG("Open(%d,%s)", a, container->sort.c_str());
    if (Plug::Open(a))
    {
        container->isOpen = true;
        UpdateTaskList("open", objects[a]);
        open_cons[a]=0;
        objects[a]->is_keep=0;
        return 1;
    }
    return 0;
}
bool RDFW::FromPlate(unsigned int a)
{   
    LOG("FromPlate(%d,%s)", a, objects[a]->sort.c_str());
    if (Plug::FromPlate(a))
    {
        SetHold(plate);
        SetPlate(nullptr);
        return 1;
    }
    fromplate_cons[a]=0;
    return 0;
}
bool RDFW::ToPlate(unsigned int a)
{
    LOG("ToPlate(%d,%s)", a, objects[a]->sort.c_str());
    if (Plug::ToPlate(a))
    {
        SetPlate(hold);
        SetHold(nullptr);
        toplate_cons[a]=0;
        return 1;
    }
    return 0;
}

bool RDFW::PutDown(unsigned int a)
{
    LOG("PutDown(%d,%s)", a, objects[a]->sort.c_str());
    if (Plug::PutDown(a))
    {
        SetHold(nullptr);
        UpdateTaskList("putdown", objects[a]);
        putdown1_cons[a]=0;
        EnsureLocationCapacity(location);
        putdown_cons[a][location]=0;
        return 1;
    }
    return 0;
}
bool RDFW::PickUp(unsigned int a)
{
    auto small = ObjectPtrCast<SmallObject>(objects[a]);
    if (!small) return 0;
    LOG("PickUp(%d,%s)", small->id, small->sort.c_str());
    if (Plug::PickUp(a))
    {
        SetHold(small);
        UpdateTaskList("pickup", objects[a]);
        pickup_cons[a]=0;
        return 1;
    }
    return 0;
}


bool RDFW::Move(unsigned int a)
{
    LOG("Move(%d)", a);

    // 1) 位置边界检查（使用动态数组大小）
    if ((int)a == UNKNOWN || (int)a < 0) {
        LOG(RED "Move: invalid target loc=%d (UNKNOWN or negative)\n" RESET, (int)a);
        return 0;
    }
    
    // 2) 确保数组容量足够，动态扩展
    EnsureLocationCapacity(a);

    // 3) 当前任务 X[0] 是否存在（聚合阶段常常不存在）
    const bool has_taskX0 =
        (task_index < tasks.size() &&
         !tasks[task_index].X.empty() &&
         tasks[task_index].X[0] != nullptr);

    auto safe_idx = [&](unsigned id)->bool {
        return (id > 0 && id < objects.size() && objects[id] != nullptr);
    };
    auto hit_move_cons = [&](unsigned id, unsigned loc)->bool {
        if (!safe_idx(id)) return false;
        if ((int)loc < 0 || (int)loc >= (int)rightlocation.size()) return false;
        // 检查二维数组边界
        if (id >= move_cons.size() || loc >= move_cons[id].size()) return false;
        return move_cons[id][loc] != 0;
    };

    // 4) 跨区前的"自清理"：手持/托盘若会触发 move 约束，先放下/取下
    if (hold_id > 0 && hit_move_cons(hold_id, a)) {
        if (!has_taskX0 || tasks[task_index].X[0]->id != hold_id) {
            PutDown(hold_id);
        }
    }
    if (plate_id > 0 && hit_move_cons(plate_id, a)) {
        if (hold_id > 0) PutDown(hold_id);
        FromPlate(plate_id);
        if (hold_id > 0) PutDown(hold_id);
    }

    // 5) 真正移动
    if (!Plug::Move(a)) {
        LOG(RED "Move: Plug::Move(%d) failed\n" RESET, (int)a);
        return 0;
    }

    // 6) 成功后的状态更新（全部带边界/判空）
    location = a;
    if (hold)  hold->location  = a;
    if (plate) plate->location = a;
    
    /*
    // 6) 移动到新位置后立即进行感知，更新物体位置信息
    // 改进：每次移动后都进行感知，更新物体位置和状态信息
    // 在多goto任务模式下跳过Sense操作
    if (!isMultiGotoMode) {
        SenseCurrentLocationOnly();
    } else {
       
    }
    */
    EnsureLocationCapacity(a);
    goto_cons[a] = 0;                          // a 边界在上面已保证
    if (hold_id > 0 && safe_idx(hold_id)) {
        move_cons[hold_id][a] = 0;
        objects[hold_id]->is_keep = 0;
    }

    // 正确地把“当前任务的目标对象”传给 UpdateTaskList
    if (has_taskX0) {
        UpdateTaskList("goto", tasks[task_index].X[0]);  // ✅ 用任务里的那个对象
    }
    return 1;
}






/*====================== 解析环境 =========================== */


void RDFW::PrintEnv()
 {
 #ifdef __DEBUG__
     vector<vector<shared_ptr<Object>>> objPos;  // 2-dim shared_ptr vector
                                                 // Each position contains all objects_ptrs
     vector<shared_ptr<Object>> unknownPos;      // 1-dim shared_ptr vector
     
     // Check each object's position and put into objPos
     for (auto v : objects)
     {
         if (v->location == UNKNOWN)
         {
             unknownPos.push_back(v);
             continue;
         }
         if (v->location >= objPos.size())
             objPos.resize(v->location + 1); // expand objPos
         objPos[v->location].push_back(v);
     }
 //把“位置正确性”的调试打印改为“按位置维度”访问
    if ((int)posCorrectFlag.size() < (int)objPos.size()) {
        posCorrectFlag.resize(objPos.size(), true);  // 统一按“位置维度”
    }

     for (int i = 0; i < objPos.size(); i++)
     {
         // print: position and is_correct
         bool is_corr = (i >= 0 && i < (int)posCorrectFlag.size()) ? (posCorrectFlag[i] == true) : true;
         cout << "Pos " << (i < 10 ? " " : "") << i << ":" << (is_corr ? "(T)" : "(F)") << ":";
         
         // print objects info in this position
         for (auto v : objPos[i])
         {
             // print robot info (green) (hold, plate)
             if (v->sort == "robot")
                 cout << GREEN << "(" << v->sort << " hold:" << hold_id << " plate:" << plate_id << ")" << RESET;
             
             // print Big Object info (yellow) (id, sort)
             else if (dynamic_pointer_cast<BigObject>(v))
             {
                 auto p = dynamic_pointer_cast<BigObject>(v);
                 cout << YELLOW << "(" << p->id << " " << p->sort;
                 // check container
                 if (dynamic_pointer_cast<Container>(v))
                 {
                     auto p = dynamic_pointer_cast<Container>(v);
                     cout << " inside:[";
                     for (int c = 0; c < p->smallObjectsInside.size(); c++)
                         cout << (c == 0 ? "" : ",") << p->smallObjectsInside[c]->id;
                     cout << "] " << (p->isOpen ? "Open" : "Closed");
                 }
                 cout << ")" << RESET;
             }
             else
                 cout << "(" << v->id << " " << v->sort << ")";
         }
         cout << endl;
     }
 
     // print Unknown Position (id, sort)
     cout << "UnknownPos:" << endl;
     for (auto v : unknownPos)
     {
         cout << BLUE << "(" << v->id << " " << v->sort << ")" << RESET << " ";
     }

     cout << endl;
 #endif
 }
 

bool RDFW::ParseInstruction(const string &taskDis) // 改并且新增两个函数
{
    if (taskDis.empty())
    {
        LOG_ERROR("Instruction is null");
        return false;
    }

    shared_ptr<SyntaxNode> root = make_shared<SyntaxNode>();
    vector<shared_ptr<SyntaxNode>> leaf_path;
    shared_ptr<SyntaxNode> curr_leaf = root;
    leaf_path.push_back(curr_leaf);
    int tag1 = 0;

    for (int i = 0; i < taskDis.size(); i++)
    {
        if (taskDis[i] == '(')
        {
            const string value = ExtractValue(taskDis, tag1, i);
            curr_leaf->value += value;
            tag1 = i + 1;
            auto p = make_shared<SyntaxNode>();
            curr_leaf->sons.emplace_back(p);
            curr_leaf = p;
            leaf_path.push_back(curr_leaf);
        }
        else if (taskDis[i] == ')')
        {
            const string value = ExtractValue(taskDis, tag1, i);
            curr_leaf->value += value;
            tag1 = i + 1;
            curr_leaf = *(leaf_path.end() - 2);
            leaf_path.pop_back();
        }
    }
    ExtractInstructions(root->sons[0]->sons);
    return true;
}



string RDFW::ExtractValue(const string &taskDis, int tag1, int tag2)
{
    const string value = taskDis.substr(tag1, tag2 - tag1);
    return (value.back() == ' ' ? value.substr(0, value.size() - 1) : value);
}



void RDFW::ExtractInstructions(const vector<shared_ptr<SyntaxNode>> &nodes)
{
    // instructions.reserve(nodes.size());
    for (const auto &node : nodes)
    {
        string instructionType = node->value;

        if (instructionType == ":task")
        {
            tasks.emplace_back(Instruction(node, shared_from_this())); // 创建一个新的 Instruction 对象
        }
        else if (instructionType == ":cons_not")
        {
            for (auto y : node->sons)
            {
                if (y->value == ":info") not_infoConstrains.emplace_back(Instruction(node->sons[0], shared_from_this()));
                if (y->value == ":task")not_taskConstrains.emplace_back(Instruction(node->sons[0], shared_from_this()));
            }
        }
        else if (instructionType == ":cons_notnot")notnot_infoConstrains.emplace_back(Instruction(node->sons[0], shared_from_this())); // 创建一个新的 Instruction 对象
        else if (instructionType == ":info")infos.emplace_back(Instruction(node, shared_from_this()));
        
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief   Parse Natural Language
 * @param   src (const): All env strings in natural language
 * @returns None
 */
void RDFW::ParseNaturalLanguage(const string &src) // 
{
    int lp = 0;
    for (int i = 0; i < src.size(); i++)
    {
        if (src[i] == '.')  // separated by sentence "."
        {
            string str = src.substr(lp, (i + 1) - lp);  // extract sentence (lp:start,  i:end)
            LOG(GREEN "%s" RESET, str.c_str()); // print sentence in log
            ParseNaturalLanguageSentence(str);  // parse sentence info
            lp = i + 2;
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief   Parse Natural Language (Single Sentence)
 * @param   s (string): single sentence to parse
 * @return  if Parse successfully
 */
bool RDFW::ParseNaturalLanguageSentence(const string &s) // 改
{
    if (!nlp_parser->parse(s))  // Fail to parse
    {
        errorlist.push_back(s);
        return false;
    }

    auto tree = nlp_parser->root;
    vector<Instruction> *list_p = nullptr;
    bool is_task = true;

    switch (tree->sons.size())
    {
    case 1:
    {
        if (tree->sons[0]->token.type == VP)
        {
            if (nlp_parser->is_not)
                list_p = &not_taskConstrains;
            else
                list_p = &tasks;
        }
        break;
    }
    case 2:
    case 3:
    {
        is_task = false;

        // 定位指令类被
        if (nlp_parser->is_must)
        {
            if (nlp_parser->is_not)
                list_p = &not_infoConstrains;
            else
                list_p = &notnot_infoConstrains;
        }
        else
            list_p = &infos;
        break;
    }
    default:
        break;
    }

    if (list_p == nullptr)
    {
        LOG_ERROR("NLP Parse Error");
    }
    else
    {
        Instruction instr;
        // 捕获task或info, 封装进Instruction
        if (is_task)
            instr = nlp_parser->get_task_instruction();
        else
            instr = nlp_parser->get_info_instruction();

        // 添加至Instruction vector
        list_p->push_back(instr);
        list_p->back().SearchConditionObject(shared_from_this(), nlp_parser->is_every);
    }

    return true;
}



bool RDFW::ParseEnvSentence(const string &sentence) // 改
{
    int pos = 1;
    vector<string> tokenList;
    tokenList.reserve(3); // Reserve space for up to 3 tokens

    // Extract token
    for (int i = 1; i < sentence.size(); i++)
    {
        if (sentence[i] == ' ' || sentence[i] == ')')
        {
            tokenList.emplace_back(&sentence[pos], &sentence[i]);
            pos = i + 1;
        }
    }

    if (tokenList.size() != 2 && tokenList.size() != 3)
    {
        LOG_ERROR("Env Sentence (%s) error", sentence.c_str());
        return false;
    }

    const string &firstToken = tokenList[0];
    int index = stoi(tokenList[1]);
    int lastSize = objects.size();

    // set robot status
    if (firstToken == "hold")
    {
        this->hold_id = index;
    }
    else if (firstToken == "plate")
    {
        this->plate_id = index;
    }
    else
    {
        while (index >= lastSize)
        {
            objects.emplace_back(make_shared<Object>(lastSize++));
            posCorrectFlag.push_back(!isErrorCorrection);
        }

        auto &obj = objects[index];

        if (firstToken == "opened" || firstToken == "closed")
        {
            auto containerPtr = dynamic_pointer_cast<Container>(obj);
            if (containerPtr == nullptr)
            {
                containerPtr = make_shared<Container>(obj);
                obj = containerPtr;
            }
            containerPtr->isOpen = (firstToken == "opened");
        }
        else if (firstToken == "at")
        {
            int L = stoi(tokenList[2]);
            EnsureLocationCapacity(L);
            obj->location = L;
            rightlocation[L] = 1;
        }
        else if (firstToken == "sort")
        {
            obj->sort = tokenList[2];
        }
        else if (firstToken == "size")
        {
            if (tokenList[2] == "big")
            {
                if (dynamic_pointer_cast<BigObject>(obj) == nullptr)
                {
                    auto bigObjPtr = make_shared<BigObject>(obj);
                    obj = bigObjPtr;
                }
            }
            else if (tokenList[2] == "small")
            {
                if (dynamic_pointer_cast<SmallObject>(obj) == nullptr)
                {
                    auto smallObjPtr = make_shared<SmallObject>(obj);
                    smallObjects.push_back(smallObjPtr);
                    obj = smallObjPtr;
                }
            }
        }
        else if (firstToken == "color")
        {
            auto smallObjPtr = dynamic_pointer_cast<SmallObject>(obj);
            if (smallObjPtr == nullptr)
            {
                smallObjPtr = make_shared<SmallObject>(obj);
                smallObjects.push_back(smallObjPtr);
                obj = smallObjPtr;
            }
            smallObjPtr->color = tokenList[2];
        }
        else if (firstToken == "inside")
        {
            auto smallObjPtr = dynamic_pointer_cast<SmallObject>(obj);
            if (smallObjPtr == nullptr)
            {
                smallObjPtr = make_shared<SmallObject>(obj);
                smallObjects.push_back(smallObjPtr);
                obj = smallObjPtr;
            }
            smallObjPtr->inside = stoi(tokenList[2]);
        }
        else if (firstToken == "type" && tokenList[2] == "container")
        {
            if (dynamic_pointer_cast<Container>(obj) == nullptr)
            {
                auto containerPtr = make_shared<Container>(obj);
                obj = containerPtr;
            }
        }
    }

    return true;
}



bool RDFW::ParseEnv(const string &env) // 没改
{
    regex reg("\\(.*?\\)");
    cmatch m;
    auto pos = env.data() + 1;
    auto end = env.data() + env.size();

    // separate sentence and parse
    for (; regex_search(pos, end, m, reg); pos = m.suffix().first)
    {
        string str = m.str();
        if (ParseEnvSentence(str) == false)
        {
            LOG_ERROR("Parse Env Sentence ERROR\n %s", env.c_str());
            return false;
        }
    }

    // set robot status
    if (hold_id > 0)
        SetHold(ObjectPtrCast<SmallObject>(objects[hold_id]));
    if (plate_id > 0)
        SetPlate(ObjectPtrCast<SmallObject>(objects[plate_id]));

    for (const auto &s : smallObjects)
    {
        if (s == plate || s == hold)
            continue;
        if (s->inside != UNKNOWN && s->inside != NONE)
        {
            auto p = dynamic_pointer_cast<Container>(objects[s->inside]);
            if (p != nullptr)
            {
                p->smallObjectsInside.push_back(s);
                s->location = p->location;
            }
            else
            {
                s->location = UNKNOWN;
                s->inside = UNKNOWN;
            }
        }
        else if (s->location != UNKNOWN)
            s->inside = NONE;
    }

    return true;
}



void RDFW::ParseInfo(const Instruction &info) // 改
{
    // 跳过包含不存在物体的info
    if (info.hasMissingObjects) {
        return;
    }
    
    const string &behave = info.behave;

    if (behave == "on")
    {
        int thelocation = info.Y[0]->location;
        for (auto v : info.X)
        {
            v->location = thelocation;
            auto small=dynamic_pointer_cast<SmallObject>(v);
            if(small!=nullptr) {
                small->inside=NONE;
                small->on = info.Y[0]->id;
            }
        }
    }
    else if (behave == "near")
    {
        int yLocation = info.Y[0]->location;
        int xLocation = info.X[0]->location;

        if (yLocation != UNKNOWN)
        {
            for (auto v : info.X)
            {    
            v->location = yLocation;
            auto small=dynamic_pointer_cast<SmallObject>(v);
            if(small!=nullptr) small->inside=NONE;
            }
            
        }
        else if (xLocation != UNKNOWN)
        {
            for (auto v : info.Y)
            {    
            v->location = xLocation;
            auto small=dynamic_pointer_cast<SmallObject>(v);
            if(small!=nullptr) small->inside=NONE;
            }
        }
    }
    else if (behave == "plate")
    {
        if (plate == nullptr)
        {
            SetPlate(ObjectPtrCast<SmallObject>(info.X[0]));
        }
        else
        {
            LOG_ERROR("The plate already has a small object (%d %s)", plate->id, plate->sort.c_str());
        }
    }
    else if (behave == "inside" || behave == "in")
    {
        auto c = ObjectPtrCast<Container>(info.Y[0]);
        int cId = c->id;

        for (auto v : info.X)
        {
            auto p = ObjectPtrCast<SmallObject>(v);
            p->inside = cId;
            p->location=c->location;
            c->smallObjectsInside.push_back(p);
        }
    }
    else if (behave == "opened")
    {
        for (auto v : info.X)
        {
            auto p = ObjectPtrCast<Container>(v);
            p->isOpen = true;
        }
    }
    else if (behave == "closed")
    {
        for (auto v : info.X)
        {
            auto p = ObjectPtrCast<Container>(v);
            p->isOpen = false;
        }
    }
}






/*====================== 工具函数=========================== */
//UpdateTaskList、LogInstructionError、AfterSolveTask、PrintInstruction、


//更新任务列表
void RDFW::UpdateTaskList(const string& behave,
        const shared_ptr<Object>& x,
        const shared_ptr<Object>& y)
    {
    for (auto& t : tasks) {
    if (!t.isEnable) continue;

    if (behave == "goto") {
    // 仅当“同一物体”时才去重（严格按对象 id）
    if (t.behave == "goto" && t.X.size() > 0 && t.X[0] && x && (t.X[0]->id == x->id)) {
    t.isEnable = false;
    }
    // 注意：这里直接 continue，避免 goto 走到下面“通用等价”分支
    continue;
    }

    // 非 goto：保持你原有的等价判定（条件/对象等）
    if (t.IsInstructionInvoke(behave, x, y)) {
    t.isEnable = false;
    }
    }
}

//记录错误任务
void RDFW::LogInstructionError(const Instruction &task) // 新增
{
    stringstream ss;
    ss << task;
    LOG_ERROR("Instruction Error\n %s", ss.str().c_str());
}

//执行完任务后更新约束
void RDFW::AfterSolveTask(const Instruction &task){
    if(task.behave=="pickup") for(int i=0;i<(int)pickup_cons.size();i++) pickup_cons[i]+=2;
    else if(task.behave=="putdown") pickup_cons[task.X[0]->id]+=2;
    else if(task.behave=="takeout") putin_cons[task.X[0]->id][task.Y[0]->id]+=2;
    else if(task.behave=="putin") takeout_cons[task.X[0]->id][task.Y[0]->id]+=2;
//这里要删除
//    else if(task.behave=="goto") for(int i=0;i<50;i++) goto_cons[i]+=2;
    else task.X[0]->is_keep+=2;
}//我还是想把这个改一下

/**
 * @brief   Print Instruction Infomation
 * @param   None
 * @note    task, info, constrains are stored as "Instruction class",
 *          include bahave, conditionX, conditionY (if it has)
 */
 void RDFW::PrintInstruction()
 {
 #ifdef __DEBUG__
     // print tasks (Do)
     cout << "Task:\n";
     for (auto v : tasks) {
         cout << v;
     }
 
     // print infos
     cout << "\nInfo:\n";
     for (auto v : infos) {
         cout << v;
     }
 
     // print not_infos (constrains)
     cout << "\nNot_Info:\n";
     for (auto v : not_infoConstrains) {
         cout << v;
     }
 
     // print not_tasks (constrains)
     cout << "\nNot_Task:\n";
     for (auto v : not_taskConstrains)
         {
         cout << v;
     }
 
     // print notnot_infos (Must keep)
     cout << "\nNotNot_Info:\n";
     for (auto v : notnot_infoConstrains)
         {
         cout << v;
     }
 #endif
 }
 
 
 
 ////////////////////////////////////////////////////////////////////////////////////////////////////////
 /**
  * @brief   Print Environment Infomation
  * @param   None
  * @note    All Env Infomations are stored in objects, include id, sort, position...
  *          we define a 2D vector to store each objects' position and its info
  * 
  */
 
 
void RDFW::Fini()
{
    cout << "#(RDFW): Fini - Comprehensive state cleanup for next test" << endl;
    
    // ==================== 机器人状态重置 ====================
    location = UNKNOWN;
    hold = nullptr;
    hold_id = 0;
    
    // ==================== 对象引用清理 ====================
    human = nullptr;
    plate = nullptr;
    plate_id = 0;
    task_index = 0;
    
    // ==================== 状态变量重置 ====================
    solved_task_num = 0;
    err_times = 0;
    isPass = false;
    isKeepConstrain = false;
    isMultiGotoMode = false;
    isAutoConstrain = false;
    isAskTwice = false;
    
    // ==================== 容器完全清理 ====================
    objects.clear();
    smallObjects.clear();
    tasks.clear();
    infos.clear();
    not_infoConstrains.clear();
    not_taskConstrains.clear();
    notnot_infoConstrains.clear();
    posCorrectFlag.clear();
    posSensedFlag.clear();
    errorlist.clear();
    lock_by_mustnear.clear();
    
    // ==================== 内存优化清理 ====================
    // 强制释放vector内存
    objects.shrink_to_fit();
    smallObjects.shrink_to_fit();
    tasks.shrink_to_fit();
    infos.shrink_to_fit();
    not_infoConstrains.shrink_to_fit();
    not_taskConstrains.shrink_to_fit();
    notnot_infoConstrains.shrink_to_fit();
    posCorrectFlag.shrink_to_fit();
    posSensedFlag.shrink_to_fit();
    errorlist.shrink_to_fit();
    lock_by_mustnear.shrink_to_fit();
    
    // ==================== 数组完全重置 ====================
    // 重置并查集数组
    memset(uf_parent, -1, sizeof(uf_parent));
    memset(uf_size, 0, sizeof(uf_size));
    memset(uf_groupLoc, -1, sizeof(uf_groupLoc));
    
    // 重置配置标志
    enable_near_correction = true;
    enable_must_lock = true;
    sense_cb = nullptr;
    
    // ==================== 动态数组深度清理 ====================
    // 清理一维数组
    fill(goto_cons.begin(), goto_cons.end(), 0);
    fill(putdown1_cons.begin(), putdown1_cons.end(), 0);
    fill(open_cons.begin(), open_cons.end(), 0);
    fill(close_cons.begin(), close_cons.end(), 0);
    fill(pickup_cons.begin(), pickup_cons.end(), 0);
    fill(givehuman_cons.begin(), givehuman_cons.end(), 0);
    fill(fromplate_cons.begin(), fromplate_cons.end(), 0);
    fill(toplate_cons.begin(), toplate_cons.end(), 0);
    fill(rightlocation.begin(), rightlocation.end(), false);
    
    // 清理二维数组
    for (auto& row : putin_cons) fill(row.begin(), row.end(), 0);
    for (auto& row : takeout_cons) fill(row.begin(), row.end(), 0);
    for (auto& row : putdown_cons) fill(row.begin(), row.end(), 0);
    for (auto& row : move_cons) fill(row.begin(), row.end(), 0);
    for (auto& row : mustnear_cons) fill(row.begin(), row.end(), 0);
    
    // 清理任务查找表
    for (auto& row : takeout) fill(row.begin(), row.end(), nullptr);
    for (auto& row : putin) fill(row.begin(), row.end(), nullptr);
    fill(close.begin(), close.end(), nullptr);
    fill(open.begin(), open.end(), nullptr);
    fill(pickup.begin(), pickup.end(), nullptr);
    fill(putdown.begin(), putdown.end(), nullptr);
    
    // ==================== 感知状态完全重置 ====================
    posSensedFlag.resize(100, false);
    locationSensedObjects.resize(100);
    
    // 深度清理位置感知数据
    for (auto& loc_info : locationSensedObjects) {
        loc_info.object_ids.clear();
        loc_info.object_ids.shrink_to_fit();
        loc_info.container_id = 0;
        loc_info.has_container = false;
    }
    
    // ==================== 解析器状态清理 ====================
    if (nlp_parser) {
        parser::clear_static_state();
    }
    
    // ==================== 重新初始化基础状态 ====================
    objects.push_back(shared_from_this());
    if (isErrorCorrection)
        posCorrectFlag.push_back(false);
    else
        posCorrectFlag.push_back(true);
    
    // ==================== 内存优化 ====================
    OptimizeMemoryUsage();
    
    cout << "#(RDFW): Comprehensive state cleanup completed - ready for next test" << endl;
}

void RDFW::OptimizeMemoryUsage() {
    cout << "#(RDFW): Optimizing memory usage..." << endl;
    
    // 强制释放所有vector的未使用内存
    objects.shrink_to_fit();
    smallObjects.shrink_to_fit();
    tasks.shrink_to_fit();
    infos.shrink_to_fit();
    not_infoConstrains.shrink_to_fit();
    not_taskConstrains.shrink_to_fit();
    notnot_infoConstrains.shrink_to_fit();
    posCorrectFlag.shrink_to_fit();
    posSensedFlag.shrink_to_fit();
    errorlist.shrink_to_fit();
    lock_by_mustnear.shrink_to_fit();
    
    // 优化动态数组内存
    goto_cons.shrink_to_fit();
    putdown1_cons.shrink_to_fit();
    open_cons.shrink_to_fit();
    close_cons.shrink_to_fit();
    pickup_cons.shrink_to_fit();
    givehuman_cons.shrink_to_fit();
    fromplate_cons.shrink_to_fit();
    toplate_cons.shrink_to_fit();
    rightlocation.shrink_to_fit();
    
    // 优化二维数组内存
    for (auto& row : putin_cons) row.shrink_to_fit();
    for (auto& row : takeout_cons) row.shrink_to_fit();
    for (auto& row : putdown_cons) row.shrink_to_fit();
    for (auto& row : move_cons) row.shrink_to_fit();
    for (auto& row : mustnear_cons) row.shrink_to_fit();
    
    // 优化任务查找表内存
    for (auto& row : takeout) row.shrink_to_fit();
    for (auto& row : putin) row.shrink_to_fit();
    close.shrink_to_fit();
    open.shrink_to_fit();
    pickup.shrink_to_fit();
    putdown.shrink_to_fit();
    
    // 优化位置感知数据内存
    for (auto& loc_info : locationSensedObjects) {
        loc_info.object_ids.shrink_to_fit();
    }
    locationSensedObjects.shrink_to_fit();
    
    cout << "#(RDFW): Memory optimization completed" << endl;
}
 
 
 
 void split_string(vector<string> &out, const string &str_source, char mark)
 {
     int last = 0;
     for (int i = 0; i < str_source.size(); i++)
     {
         if (str_source[i] == mark)
         {
             out.push_back(str_source.substr(last, i - last));
             last = i + 1;
         }
     }
     if (last != str_source.size())
         out.push_back(str_source.substr(last, str_source.size() - 1));
 }
 
 
 
 

 Instruction::Instruction() {}
 
 Instruction::Instruction(const shared_ptr<SyntaxNode> &node, const shared_ptr<RDFW> &rdfw) // 改
 {
     vector<string> disc;
     split_string(disc, node->sons[0]->value, ' '); // node是ins下一层的小节点
     behave = disc[0];
 
    for (const auto &n : node->sons[1]->sons) // sons[1]是cond节点
    {
        vector<string> temp;
        split_string(temp, n->value, ' ');

        // 检查temp向量是否有足够的元素
        if (temp.size() < 3) {
            LOG_ERROR("Invalid condition format: %s", n->value.c_str());
            continue;
        }

        Condition *con = &conditionX;
        if (temp[1] == "Y")
        {
            isUseY = true;
            con = &conditionY;
        }
        if (temp[0] == "color")
        {
            con->color = temp[2];
        }
        else if (temp[0] == "sort")
        {
            con->sort = temp[2];
        }
    }
 
     SearchConditionObject(rdfw);
 }
 
 
 ostream &operator<<(ostream &os, const Instruction &instr)
 {
     os << instr.ToString();
     return os;
 }
 
 ostream &operator<<(ostream &os, shared_ptr<SyntaxNode> sn)
 {
     static int layer = 0;
     for (int i = 0; i < layer; i++)
         os << "-";
     os << sn->value << '|' << endl;
     layer++;
     for (int i = 0; i < sn->sons.size(); i++)
     {
         os << sn->sons[i];
     }
     layer--;
     return os;
 }
 
 ostream &operator<<(ostream &os, shared_ptr<Object> obj)
 {
     if (dynamic_pointer_cast<SmallObject>(obj) != nullptr)
     {
         os << "SmallObject " << dynamic_pointer_cast<SmallObject>(obj)->ToString();
     }
     else if (dynamic_pointer_cast<Robot>(obj) != nullptr)
     {
         os << "this " << dynamic_pointer_cast<Robot>(obj)->ToString();
     }
     else if (dynamic_pointer_cast<BigObject>(obj) != nullptr)
     {
         if (dynamic_pointer_cast<Container>(obj) != nullptr)
         {
             os << "Container " << dynamic_pointer_cast<Container>(obj)->ToString();
         }
         else
         {
             os << "BigObject  " << dynamic_pointer_cast<BigObject>(obj)->ToString();
         }
    }
    return os;
}

 /**
  * @brief Must Near 纠错与补全（极简版）
  * - 用并查集把 near 约束形成的对象连通分量合并
  * - 对每个分量按“已知位置”的多数票决定组位置
  * - 如果启用上锁，则把组内所有对象位置改为该“组位置”，并标记为锁定
  * 依赖成员：
  *   objects, notnot_infoConstrains, lock_by_mustnear, enable_near_correction, enable_must_lock, UNKNOWN
  */
 void RDFW::ApplyMustNearConstraintCorrection(){

    const int numObjs = static_cast<int>(objects.size());

     if (!enable_near_correction) {
         LOG(YELLOW "[MustNear] disabled\n" RESET);
         return;
     }

     if (numObjs == 0) {
         LOG(YELLOW "[MustNear] no objects\n" RESET);
         return;
     }
 
     // 并查集：局部化，避免类里再放 uf_parent/size 等成员
     std::vector<int> parent(numObjs), sz(numObjs, 1);
     std::iota(parent.begin(), parent.end(), 0);
 
     auto find = [&](int x) {
         while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
         return x;
     };
     auto unite = [&](int a, int b) {
         a = find(a); b = find(b);
         if (a == b) return;
         if (sz[a] < sz[b]) std::swap(a, b);
         parent[b] = a; sz[a] += sz[b];
     };
 
     // 1) 收集 must near 约束并合并等价类
     for (const auto &cons : notnot_infoConstrains) {
         if ((cons.behave == "near"||cons.behave == "on"||cons.behave == "nextto") && !cons.X.empty() && !cons.Y.empty()) {
            if(cons.X[0]->id==hold_id) hold_mustnear = true;
            if(cons.X[0]->id==plate_id) plate_mustnear = true;
            if(cons.Y[0]->id==hold_id) hold_mustnear = true;
            if(cons.Y[0]->id==plate_id) plate_mustnear = true;
            if(cons.X[0]->location!=cons.Y[0]->location){
            // INSERT_YOUR_CODE
            // 分别统计cons.X[0]和cons.Y[0]的near/notnear情况
            // cons.X[0] 在它自己位置上的物体统计
            int x_near_status = 0; // 0=无, 1=有near, 2=有notnear
            int y_near_status = 0;
            int xloc = cons.X[0]->location;
            int yloc = cons.Y[0]->location;

            // 如果有对象在 xloc, 判断是否有near或notnear关系
            for (const auto& obj : objects) {
                if (!obj) continue;
                if (obj->id == cons.X[0]->id) continue;
                if (obj->location == xloc) {
                    // 判断 cons.X[0] 与 obj 的 near/notnear 关系
                    // 检查 notnot_infoConstrains
                    for (const auto& c2 : notnot_infoConstrains) {
                        if (
                            (c2.behave == "near" || c2.behave == "on"||c2.behave == "nextto") &&
                            !c2.X.empty() && !c2.Y.empty() &&
                            (
                                (c2.X[0]->id == cons.X[0]->id && c2.Y[0]->id == obj->id) ||
                                (c2.Y[0]->id == cons.X[0]->id && c2.X[0]->id == obj->id)
                            )
                        ) {
                            x_near_status = 1;
                            break;
                        }
                    }
                    // 检查 not_infoConstrains
                    for (const auto& c2 : not_infoConstrains) {
                        if (
                            (c2.behave == "near" || c2.behave == "on"||c2.behave == "nextto") &&
                            !c2.X.empty() && !c2.Y.empty() &&
                            (
                                (c2.X[0]->id == cons.X[0]->id && c2.Y[0]->id == obj->id) ||
                                (c2.Y[0]->id == cons.X[0]->id && c2.X[0]->id == obj->id)
                            )
                        ) {
                            x_near_status = 2;
                            break;
                        }
                    }
                    if (x_near_status) break;
                }
            }
            // 如果有对象在 yloc, 判断是否有near或notnear关系
            for (const auto& obj : objects) {
                if (!obj) continue;
                if (obj->id == cons.Y[0]->id) continue;
                if (obj->location == yloc) {
                    // 判断 cons.Y[0] 与 obj 的 near/notnear 关系
                    // 检查 notnot_infoConstrains
                    for (const auto& c2 : notnot_infoConstrains) {
                        if (
                            (c2.behave == "near" || c2.behave == "on"||c2.behave == "nextto") &&
                            !c2.X.empty() && !c2.Y.empty() &&
                            (
                                (c2.X[0]->id == cons.Y[0]->id && c2.Y[0]->id == obj->id) ||
                                (c2.Y[0]->id == cons.Y[0]->id && c2.X[0]->id == obj->id)
                            )
                        ) {
                            y_near_status = 1;
                            break;
                        }
                    }
                    // 检查 not_infoConstrains
                    for (const auto& c2 : not_infoConstrains) {
                        if (
                            (c2.behave == "near" || c2.behave == "on"||c2.behave == "nextto") &&
                            !c2.X.empty() && !c2.Y.empty() &&
                            (
                                (c2.X[0]->id == cons.Y[0]->id && c2.Y[0]->id == obj->id) ||
                                (c2.Y[0]->id == cons.Y[0]->id && c2.X[0]->id == obj->id)
                            )
                        ) {
                            y_near_status = 2;
                            break;
                        }
                    }
                    if (y_near_status) break;
                }
            }
            // 统计完成，如需将结果用于后续逻辑或输出可在后面使用 x_near_status, y_near_status
            if(x_near_status==2&&y_near_status!=2){
                cons.X[0]->location = cons.Y[0]->location;
                LOG(GREEN "[MustNear] set obj %d location to %d\n" RESET, cons.X[0]->id, cons.Y[0]->location);
            }
            else if(x_near_status!=2&&y_near_status==2){
                cons.Y[0]->location = cons.X[0]->location;
                LOG(GREEN "[MustNear] set obj %d location to %d\n" RESET, cons.Y[0]->id, cons.X[0]->location);
            }
             unite(cons.X[0]->id, cons.Y[0]->id);
             LOG(GREEN "[MustNear] union %u ~ %u\n" RESET, cons.X[0]->id, cons.Y[0]->id);
         }
     }
    }
     // 2) 按“已知位置”给每个组投票（root -> loc->count）
     std::unordered_map<int, std::unordered_map<int,int>> vote;
     vote.reserve(numObjs);
     for (int i = 0; i < numObjs; ++i) {
         if (!objects[i]) continue;
         const int loc = objects[i]->location;
         if (loc == UNKNOWN) continue;
         vote[find(i)][loc]++;   // root 组对 loc 投票 +1
     }
 
     // 3) 选出每个组的“多数位置”；若无人投票则保持 UNKNOWN
     std::vector<int> groupChosen(numObjs, UNKNOWN);
     for (auto &kv : vote) {
         const int root = kv.first;
         int bestLoc = UNKNOWN, bestCnt = -1;
         for (auto &lc : kv.second) {
             if (lc.second > bestCnt || (lc.second == bestCnt && lc.first < bestLoc)) {
                 bestCnt = lc.second;
                 bestLoc = lc.first; // 少数服从多数；平票取更小的 loc
             }
         }
         groupChosen[root] = bestLoc;
     }
 
     // 4)（可选）上锁：把一个组内的所有对象位置改到"多数位置"，并打锁位标记
     lock_by_mustnear.assign(numObjs, false);
     if (enable_must_lock) {
         for (int i = 0; i < numObjs; ++i) {
             const int root = find(i);
             const int loc  = groupChosen[root];
             if (loc == UNKNOWN) continue;  // 该组没有确定位置，跳过
             if (!objects[i]) continue;
             objects[i]->location   = loc;
             lock_by_mustnear[i]    = true;
             LOG(GREEN "[MustNear] lock obj[%d] @ %d\n" RESET, i, loc);
         }
     }
     LOG(GREEN "[MustNear] correction done.\n" RESET);
}
 

void RDFW::ApplyMustInConstraintCorrection() {
    const int numObjs = static_cast<int>(objects.size());
    if (numObjs <= 0) return;

    // 采集 mustin（或同义）约束：记录每个物体唯一的容器
    std::vector<int> obj_to_cont(numObjs, -1);
    for (const auto& cons : notnot_infoConstrains) {
        if (cons.behave == "inside"||cons.behave == "in" && !cons.X.empty() && !cons.Y.empty())
        {
            const int x = static_cast<int>(cons.X[0]->id);
            const int y = static_cast<int>(cons.Y[0]->id);
            if (x < 0 || x >= numObjs || y < 0 || y >= numObjs) continue;
            cout<<"x: "<<x<<" y: "<<y<<endl;

            obj_to_cont[x] = y;

            cout<<"obj_to_cont[x]: "<<obj_to_cont[x]<<endl;

            // 直接同步smallObject和container信息
            if (objects[x]) {
                if (auto sm = std::dynamic_pointer_cast<SmallObject>(objects[x])) {
                    cout<<"sm->inside: "<<sm->inside<<endl;
                    // 移除在原容器中的记录
                    if (sm->inside != UNKNOWN && sm->inside != y) {
                        if (sm->inside >= 0 && sm->inside < numObjs) {
                            if (auto old_cont = std::dynamic_pointer_cast<Container>(objects[sm->inside])) {
                                auto& vec = old_cont->smallObjectsInside;
                                vec.erase(std::remove_if(vec.begin(), vec.end(),
                                    [&](const std::shared_ptr<SmallObject>& ptr) {
                                        return ptr && ptr->id == sm->id;
                                    }), vec.end());
                            }
                        }
                    }
                    sm->inside = y;
                    // 添加到目标容器（避免重复）
                    if (objects[y]) {
                        if (auto cont = std::dynamic_pointer_cast<Container>(objects[y])) {
                            bool exists = false;
                            for (auto&& v : cont->smallObjectsInside) {
                                if (v && v->id == sm->id) { exists = true; break; }
                            }
                            if (!exists)
                                cont->smallObjectsInside.push_back(sm);
                        }
                    }
                }
            }
            // 同步位置（取容器已知位置为准，否则不强制）
            if (objects[x] && objects[y]) {
                int y_loc = objects[y]->location;
                if (y_loc != UNKNOWN) {
                    objects[x]->location = y_loc;
                    LOG(GREEN "[MustIn] (direct) set obj %d @ %d (in %d)\n" RESET, x, y_loc, y);
                }
            }
        }
    }

    // INSERT_YOUR_CODE
    // 处理 notinside 约束的纠错与补全
    // 遍历所有 notnot_infoConstrains，查找 behave == "notinside" 的约束
    for (const auto& cons : not_infoConstrains) {
        if (cons.behave != "inside"&&cons.behave != "in") continue;
        if (cons.X.empty() || cons.Y.empty()) continue;
        int x = static_cast<int>(cons.X[0]->id);   // smallObject id
        int y = static_cast<int>(cons.Y[0]->id);   // container id
        if (x < 0 || x >= numObjs || y < 0 || y >= numObjs) continue;
        // 防御性检查
        auto smObj = std::dynamic_pointer_cast<SmallObject>(objects[x]);
        if (!smObj) continue;
        // 如果x当前就在y里，需要移除
        if (smObj->inside == y) {
            // 修改smallObject的inside信息
            smObj->inside = UNKNOWN;
            int old_loc = smObj->location;
            smObj->location = UNKNOWN;
            // 同时尝试从container的smallObjectsInside中移除
            auto cont = std::dynamic_pointer_cast<Container>(objects[y]);
            if (cont) {
                auto& vec = cont->smallObjectsInside;
                vec.erase(std::remove_if(vec.begin(), vec.end(),
                    [&](const std::shared_ptr<SmallObject>& ptr){
                        return ptr && ptr->id == smObj->id;
                    }), vec.end());
            }
            LOG(GREEN "[NotInside] remove obj %d from container %d\n" RESET, x, y);
        }
        // 如果inside本来就不是y，例如 UNKNOWN 或其他容器，则无需操作
    }

    LOG(GREEN "[MustIn] correction done.\n" RESET);
}

// open和close的纠正和补全，只处理not_infoConstrains和notnot_infoConstrains

void RDFW::ApplyOpenCloseCorrection() {
    // 遍历所有容器，对opened/closed的约束进行处理
    for (const auto& obj : objects) {
        if (!obj) continue;
        auto cont = std::dynamic_pointer_cast<Container>(obj);
        if (!cont) continue;

        // 收集是否存在opened/closed的约束
        bool must_open = false;
        bool must_closed = false;
        bool cons_not_open = false;
        bool cons_not_closed = false;

        // notnot_infoConstrains：必须成立
        for (const auto& cons : notnot_infoConstrains) {
            if (cons.X.empty()) continue;
            if (cons.X[0]->id != obj->id) continue;
            if (cons.behave == "opened") must_open = true;
            else if (cons.behave == "closed") must_closed = true;
        }
        // not_infoConstrains：必须不成立
        for (const auto& cons : not_infoConstrains) {
            if (cons.X.empty()) continue;
            if (cons.X[0]->id != obj->id) continue;
            if (cons.behave == "opened") cons_not_open = true;
            else if (cons.behave == "closed") cons_not_closed = true;
        }

        // 优先满足冲突最小原则
        // 如果必须open且不能closed
        if (must_open && !cons_not_open && !must_closed) {
            if (!cont->isOpen) {
                cont->isOpen = true;
                LOG(GREEN "[AutoOpenClose] set container %d open (by notnot)\n" RESET, cont->id);
            }
        }
        // 如果必须closed且不能open
        else if (must_closed && !cons_not_closed && !must_open) {
            if (cont->isOpen) {
                cont->isOpen = false;
                LOG(GREEN "[AutoOpenClose] set container %d closed (by notnot)\n" RESET, cont->id);
            }
        }
        // 如果not要求not opened，则要关闭
        else if (cons_not_open && !must_open) {
            if (cont->isOpen) {
                cont->isOpen = false;
                LOG(GREEN "[AutoOpenClose] set container %d closed (by not constraint)\n" RESET, cont->id);
            }
        }
        // 如果not要求not closed，则要打开
        else if (cons_not_closed && !must_closed) {
            if (!cont->isOpen) {
                cont->isOpen = true;
                LOG(GREEN "[AutoOpenClose] set container %d open (by not constraint)\n" RESET, cont->id);
            }
        }
        // 冲突：有not和notnot都要求open/closed
        else if ((must_open && cons_not_open) || (must_closed && cons_not_closed) || (must_open && must_closed)) {
            // 默认优先closed
            cont->isOpen = false;
            LOG(YELLOW "[AutoOpenClose] conflict: container %d open/close constraints conflict, set to closed\n" RESET, cont->id);
        }
        // 无约束不处理
    }
}




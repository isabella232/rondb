/*
   Copyright (c) 2003, 2019, Oracle and/or its affiliates. All rights reserved.
   Copyright (c) 2021, 2021, Logical Clocks AB and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef Configuration_H
#define Configuration_H

#include <ndb_global.h>

#include <util/BaseString.hpp>
#include <mgmapi.h>
#include <kernel_types.h>
#include <NdbMutex.h>
#include <NdbThread.h>
#include <util/SparseBitmask.hpp>
#include <util/UtilBuffer.hpp>
#include "mt_thr_config.hpp"

#define JAM_FILE_ID 276


enum ThreadTypes
{
  WatchDogThread = 1,
  SocketServerThread = 2,
  SocketClientThread = 3,
  NdbfsThread = 4,
  BlockThread = 5,
  SendThread = 6,
  ReceiveThread = 7,
  NotInUse = 8
};

#define MAX_NDB_THREADS 256
#define NO_LOCK_CPU 0x10000

struct ThreadInfo
{
  enum ThreadTypes type;
  struct NdbThread* pThread;
};

class ConfigRetriever;

class Configuration {
public:
  Configuration();
  ~Configuration();

  bool init(int _no_start, int _initial,
            int _initialstart);


  void fetch_configuration(const char* _connect_string, int force_nodeid,
                           const char* _bind_adress,
                           NodeId allocated_nodeid,
                           int connect_retries, int connect_delay);
  void setupConfiguration();
  void closeConfiguration(bool end_session= true);
  
  Uint32 lockPagesInMainMemory() const;

  int schedulerExecutionTimer() const;
  void schedulerExecutionTimer(int value);

  int schedulerSpinTimer() const;
  void schedulerSpinTimer(int value);

  Uint32 spinTimePerCall() const;

  Uint32 maxSendDelay() const;

  Uint32 schedulerResponsiveness() const
  { return _schedulerResponsiveness; }
  void setSchedulerResponsiveness(Uint32 val)
  {
    _schedulerResponsiveness = val;
  }

  bool realtimeScheduler() const;
  void realtimeScheduler(bool realtime_on);

  Uint32 executeLockCPU() const;
  void executeLockCPU(Uint32 value);

  Uint32 maintLockCPU() const;
  void maintLockCPU(Uint32 value);

  void setAllRealtimeScheduler();
  void setAllLockCPU(bool exec_thread);
  int setLockCPU(NdbThread*, enum ThreadTypes type);
  int setThreadPrio(NdbThread*, enum ThreadTypes type);
  int setRealtimeScheduler(NdbThread*,
                           enum ThreadTypes type,
                           bool real_time,
                           bool init);
  bool get_io_real_time() const;
  Uint32 addThread(struct NdbThread*,
                   enum ThreadTypes type,
                   bool single_threaded = false);
  void removeThread(struct NdbThread*);
  void yield_main(Uint32 thread_index, bool start);
  void initThreadArray();

  int timeBetweenWatchDogCheck() const ;
  void timeBetweenWatchDogCheck(int value);
  
  int maxNoOfErrorLogs() const ;
  void maxNoOfErrorLogs(int val);

  bool stopOnError() const;
  void stopOnError(bool val);
  
  int getRestartOnErrorInsert() const;
  void setRestartOnErrorInsert(int);

#ifdef ERROR_INSERT
  Uint32 getMixologyLevel() const;
  void setMixologyLevel(Uint32);
#endif 
 
  // Cluster configuration
  const char * fileSystemPath() const;
  const char * backupFilePath() const;

  bool getInitialStart() const { return _initialStart; }

  ndb_mgm_configuration_iterator * getOwnConfigIterator() const;

  ConfigRetriever* get_config_retriever() { return m_config_retriever; }

  class LogLevel * m_logLevel;
  ndb_mgm_configuration_iterator * getClusterConfigIterator() const;

  ndb_mgm_configuration* getClusterConfig() const { return m_clusterConfig; }
  Uint32 get_config_generation() const; 

  THRConfigApplier m_thr_config;
private:
  friend class Cmvmi;
  friend class Qmgr;

  Uint32 _stopOnError;
  Uint32 m_restartOnErrorInsert;
  Uint32 _maxErrorLogs;
  Uint32 _lockPagesInMainMemory;
  Uint32 _timeBetweenWatchDogCheck;
  Uint32 _schedulerExecutionTimer;
  Uint32 _schedulerSpinTimer;
  Uint32 _spinTimePerCall;
  Uint32 _realtimeScheduler;
  Uint32 _maxSendDelay;
  Uint32 _schedulerResponsiveness;
  Uint32 _timeBetweenWatchDogCheckInitial;
#ifdef ERROR_INSERT
  Uint32 _mixologyLevel;
#endif

  Vector<struct ThreadInfo> threadInfo;
  NdbMutex *threadIdMutex;

  ndb_mgm_configuration * m_ownConfig;
  const class ConfigValues* get_own_config_values();
  ndb_mgm_configuration * m_clusterConfig;
  UtilBuffer m_clusterConfigPacked_v1;
  UtilBuffer m_clusterConfigPacked_v2;

  ndb_mgm_configuration_iterator * m_clusterConfigIter;
  ndb_mgm_configuration_iterator * m_ownConfigIterator;
  
  ConfigRetriever *m_config_retriever;

  /**
   * arguments to NDB process
   */
  char * _fsPath;
  char * _backupPath;
  bool _initialStart;

  void calcSizeAlt(class ConfigValues * );
  const char *get_type_string(enum ThreadTypes type);
  bool calculate_automatic_memory(
         ndb_mgm_configuration_iterator *p);
  void assign_default_memory_sizes(const ndb_mgm_configuration_iterator *p);
  static Uint32 get_num_threads();
  static Uint64 get_total_memory(
                  const ndb_mgm_configuration_iterator *p);
  Uint64 get_schema_memory(ndb_mgm_configuration_iterator *p);
  static Uint64 get_and_set_transaction_memory(
           const ndb_mgm_configuration_iterator *p);
  static Uint64 get_and_set_redo_buffer(
           const ndb_mgm_configuration_iterator *p);
  static Uint64 get_and_set_undo_buffer(
           const ndb_mgm_configuration_iterator *p);
  static Uint64 get_and_set_long_message_buffer(
           const ndb_mgm_configuration_iterator *p);
  static Uint64 compute_os_overhead(Uint64 total_memory);
  static Uint64 compute_static_overhead();
  static Uint64 compute_backup_page_memory(
                  const ndb_mgm_configuration_iterator *p);
  static Uint64 compute_restore_memory();
  static Uint64 compute_pack_memory();
  static Uint64 compute_fs_memory();
  static Uint64 get_and_set_shared_global_memory(
           const ndb_mgm_configuration_iterator *p);
public:
  static Uint64 get_send_buffer(const ndb_mgm_configuration_iterator *p);
  void get_num_nodes(Uint32 & noOfNodes,
                     Uint32 & noOfDBNodes,
                     Uint32 & noOfAPINodes,
                     Uint32 & noOfMGMNodes);
};

inline
const char *
Configuration::fileSystemPath() const {
  return _fsPath;
}

inline
const char *
Configuration::backupFilePath() const {
  return _backupPath;
}


#undef JAM_FILE_ID

#endif

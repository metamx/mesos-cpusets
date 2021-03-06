// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <process/process.hpp>
#include <process/subprocess.hpp>

#include "CpusetIsolator.hpp"

using namespace process;

process::Future<Nothing> CpusetIsolatorProcess::recover(
  const list<mesos::slave::ContainerState>& states,
  const hashset<mesos::ContainerID>& orphans) {

/*
  foreach (const mesos::slave::ContainerState& run, states) {
    if (pids.contains(run.container_id())) {
      return process::Failure("Container already recovered");
    }

    pids.put(run.container_id(), run.pid());

    process::Owned<process::Promise<mesos::slave::ContainerLimitation>> promise(
        new process::Promise<mesos::slave::ContainerLimitation>());
    promises.put(run.container_id(), promise);
  } */

  return Nothing();
}

process::Future<Option<mesos::slave::ContainerPrepareInfo>> CpusetIsolatorProcess::prepare(
  const mesos::ContainerID& containerId,
  const mesos::ExecutorInfo& executorInfo,
  const std::string& directory,
  const Option<std::string>& user) {

/*  if (promises.contains(containerId)) {
    return process::Failure("Container " + stringify(containerId) +
                            " has already been prepared");
  }

  process::Owned<process::Promise<mesos::slave::ContainerLimitation>> promise(
      new process::Promise<mesos::slave::ContainerLimitation>());

  promises.put(containerId, promise);
*/
  return None();

}

process::Future<Nothing> CpusetIsolatorProcess::isolate(
  const mesos::ContainerID& containerId,
  pid_t pid)
{
  if (!pids.contains(containerId)) {
    return Failure("Unknown container");
  }

  if (!containerResources.contains(containerId)) {
    return Failure("Unknown container resources");
  }

  const mesos::Resources r = containerResources[containerId];
  const double cpus = r.cpus().get();
  //const double gpus = r.gpus().get();

  create_cpuset_group(containerId.value());

  CpusetAssigner cpuSetAssigner;

  cpuSetAssigner.assign(
    containerId,
    pid,
    cpus,
    0.0);
    //gpus);

  return Nothing();
}


process::Future<mesos::slave::ContainerLimitation> CpusetIsolatorProcess::watch(
  const mesos::ContainerID& containerId)
{
/*
  if (!promises.contains(containerId)) {
    return process::Failure("Unknown container: " + stringify(containerId));
  }

  return promises[containerId]->future();
*/

  return process::Future<mesos::slave::ContainerLimitation>();
}


process::Future<Nothing> CpusetIsolatorProcess::update(
  const mesos::ContainerID& containerId,
  const mesos::Resources& resources)
{
  if(containerResources.find(containerId) == containerResources.end()) {
    containerResources.insert(make_pair(containerId, resources));
  }

  return Nothing();
}


process::Future<mesos::ResourceStatistics> CpusetIsolatorProcess::usage(
  const mesos::ContainerID& containerId)
{
  if (!pids.contains(containerId)) {
    LOG(WARNING) << "No resource usage for unknown container '"
                 << containerId << "'";
  }

  return mesos::ResourceStatistics();
}


process::Future<Nothing> CpusetIsolatorProcess::cleanup(
  const mesos::ContainerID& containerId) {
  return _cleanup(containerId);
}


process::Future<Nothing> CpusetIsolatorProcess::_cleanup(
  const mesos::ContainerID& containerId)
{
  if (!containerResources.contains(containerId)) {
    return Failure("Unknown container");
  }

  //CHECK_NOTNULL(containerResources[containerId]);

  /*if (!future.isReady()) {
    return Failure(
        "Failed to clean up container " + stringify(containerId) +
        " : " + (future.isFailed() ? future.failure() : "discarded"));
  }*/

  containerResources.erase(containerId);
  destroy_cpuset_group(containerId.value()).get();

  //return future.onAny([]() { return Nothing(); });
  return Nothing();
}

Try<mesos::slave::Isolator*> CpusetIsolatorProcess::create(
    const mesos::Parameters& parameters)
{
  return new CpusetIsolator(process::Owned<CpusetIsolatorProcess>(
      new CpusetIsolatorProcess(parameters)), true);

/*
  vector<string> args{"cp", "-aT", layer, rootfs};

  Try<Subprocess> s = subprocess(
      "cp",
      args,
      Subprocess::PATH("/dev/null"),
      Subprocess::PATH("/dev/null"),
      Subprocess::PIPE());

  if (s.isError()) {
    return Failure("Failed to create 'cp' subprocess: " + s.error());
  }

  Subprocess cp = s.get();
  pid_t pid = cp.pid();

  return cp.status()
    .then([cp](const Option<int>& status) -> process::Future<Nothing> {
      if (status.isNone()) {
        return Failure("Failed to reap subprocess to copy image");
      } else if (status.get() != 0) {
        return io::read(cp.err().get())
          .then([](const string& err) -> process::Future<Nothing> {
            return Failure("Failed to copy layer: " + err);
          });
      }

      return Nothing();
    });
*/
}

static bool compatible() {
  return true;
}


static mesos::slave::Isolator* createCpusetIsolator(const mesos::Parameters& parameters)
{
  Try<mesos::slave::Isolator* > cpusetIsolator = CpusetIsolatorProcess::create(parameters);

  if (cpusetIsolator.isError()) {
    return NULL;
  }

  return cpusetIsolator.get();
}

// Declares a module named 'org_apache_mesos_TestModule' of
// 'TestModule' kind.
// Mesos core binds the module instance pointer as needed.
// The compatible() hook is provided by the module for compatibility checks.
// The create() hook returns an object of type 'TestModule'.
mesos::modules::Module<mesos::slave::Isolator> org_apache_mesos_CpusetIsolatorProcess(
  MESOS_MODULE_API_VERSION,
  MESOS_VERSION,
  "Apache Mesos",
  "ct.clmsn@gmail.com",
  "Binds processes to cgroup cpusets using a greedy submodular set selection algorithm.",
  NULL,
  createCpusetIsolator);


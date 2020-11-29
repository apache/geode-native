# Publish Pipeline
```console
./set-pipeline.sh --help
```

## Example
Given the local repository looks like the following.
```console
$ git remote get-url origin
git@github.com:some-user/geode-native.git

$ git branch --show-current
wip/some-branch
```

### Typical
The most typical usage should require nothing more than the Concourse target, unless that happens to be named "default".
Everything else has reasonable defaults based on the currently checked out branch. 
```console
$ ./set-pipeline.sh --target=test
```
Executes `fly` from the path setting pipeline to target `test` for remote repository `git@github.com:some-user/geode-native.git`.
Pipeline name will be `some-user-wip-something` 

### Alternative repository URL and fly version
Sometimes you will have to support multiple versions of Concourse `fly` or need to fetch sources via https.
```console
$ ./set-pipeline.sh \
  --fly=/path/to/fly \
  --target=test \
  --repository=https://github.com/some-user/geode-native.git
```
Executes fly at `/path/to/fly` setting pipeline to target `test` for remote repository `https://github.com/some-user/geode-native.git`.
Pipeline name will be `some-user-wip-something` 

# Pipeline Steps
1. Creates VM instances
2. Waits for VM instance to be accessible
3. Builds and packages
4. Runs all tests
5. If anything fails it downloads the build directory for later analysis
6. Deletes the VM instances

# Details
This Concourse pipeline YAML is rendered using `ytt`. Depends on output from `git` and `gcloud`.
## Dependencies
* [Concourse](https://concourse-ci.org) v6.5.0+
* [`ytt`](https://get-ytt.io) v0.28.0+
* [`git`](https://git-scm.com) v2.25.2+
* [`gcloud`](https://cloud.google.com/sdk/docs/install) SDK

#TODO
## Concourse Installation
* helm upgrade concourse concourse/concourse --set web.service.api.type=LoadBalancer,concourse.web.externalUrl=http://35.222.132.46:8080
* kubectl create secret generic gcr-json-key --from-literal "value=$(cat ~/Downloads/gemfire-dev-6e8864f0768c.json)" --namespace=concourse-main

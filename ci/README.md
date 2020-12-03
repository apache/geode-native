# Geode Native Concourse Pipeline
The Geode Native Concourse pipeline is actually two Concourse pipelines.
The primary release pipeline builds the release artifacts for a given branch, like develop, release or support branch.
The secondary pull request (pr) pipeline builds the same artifacts as the release pipeline but for pull requests and without actually releasing or publishing anything.   

The pipeline is fully self updating and can easily be bootstrapped into a properly configured Concourse deployment.
Concourse configuration requires TBD.

# Pipeline Setup
The pipeline can be installed or reconfigured via the `set-pipelin.sh` shell script.
```console
./set-pipeline.sh --help
```

## Examples
Given the local repository looks like the following:
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
$ ./set-pipeline.sh --target=some-concourse
```
Executes `fly` from the path setting pipeline to target `some-concourse` for remote repository `git@github.com:some-user/geode-native.git`.
Pipeline names will be `some-user-wip-something` and `some-user-wip-something-pr`.

### Alternative repository URL and fly version
Sometimes you will have to support multiple versions of Concourse `fly` or need to fetch sources via https.
```console
$ ./set-pipeline.sh \
  --fly=/path/to/fly \
  --target=some-concourse \
  --repository=https://github.com/some-user/geode-native.git
```
Executes fly at `/path/to/fly` setting pipeline to target `some-concourse` for remote repository `https://github.com/some-user/geode-native.git`.
Pipelines name will be `some-user-wip-something` and `some-user-wip-something-pr`.

# Pipeline Steps
## Release
* Detects new version or source
* Build for each platform and configuration
    * Creates VM instances
    * Waits for VM instance to be accessible
    * Builds and packages
    * Runs all tests
    * If anything fails it downloads the build directory for later analysis
    * Deletes the VM instances
* Publishes to GitHub release
## Pull Release (PR)
* Detects new PR
* Build for each platform and configuration
    * Creates VM instances
    * Waits for VM instance to be accessible
    * Builds and packages
    * Runs all tests
    * If anything fails it downloads the build directory for later analysis
    * Deletes the VM instances
    * Updates PR status

# Details
This Concourse pipeline YAML is rendered using `ytt`. Depends on output from `git` and `gcloud`.
## Dependencies
* [Concourse](https://concourse-ci.org) v6.5.0+
* [`ytt`](https://get-ytt.io) v0.28.0+
* [`git`](https://git-scm.com) v2.25.2+
* [`gcloud`](https://cloud.google.com/sdk/docs/install) SDK

#TODO
## Concourse Installation
* Resolve chicken/egg problem with external API address.
  * `helm install concourse concourse/concourse`
  * `helm upgrade concourse concourse/concourse --set web.service.api.type=LoadBalancer,concourse.web.externalUrl=http://1.2.3.4:8080`
* Task for getting secrets into k8s.
  * `kubectl create secret generic gcr-json-key --from-literal "value=$(cat XXX.json)" --namespace=concourse-main`
  * `kubectl create secret generic github-access-token --from-literal "value=XXX" --namespace=concourse-main`
* Use docker locally for initial pipeline deployment to avoid `gcloud`, `ytt`, and `fly` version issues.

# Geode Native Concourse Pipeline

The Geode Native Concourse pipeline is actually two Concourse pipelines. The primary release pipeline builds the release
artifacts for a given branch, like develop, release or support branch. The secondary pull request (pr) pipeline builds
the same artifacts as the release pipeline but for pull requests and without actually releasing or publishing anything.

The pipeline is fully self updating and can easily be bootstrapped into a properly configured Concourse deployment.
Concourse configuration requires TBD. Changes to the `ci` source directory will results in auto updates to the
pipelines.

Because Concourse workers aren't available on all platforms and have issues with resource sharing this pipeline utilizes
external builders. These builders are currently Google Compute VMs that are launched on demand for each build.

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

Executes `fly` from the path setting pipeline to target `some-concourse` for GitHub owner `some-user`
repository `geode-native.git`. Pipeline names will be `some-user-wip-something`
and `some-user-wip-something-pr`.

### Alternative repository URL and fly version

Sometimes you will have to support multiple versions of Concourse `fly` or need to fetch sources via https.

```console
$ ./set-pipeline.sh \
  --fly=/path/to/fly \
  --target=some-concourse \
  --github-owner=other-user
```

Executes fly at `/path/to/fly` setting pipeline to target `some-concourse` for GitHub owner `other-user`
repository `geode-native.git`. Pipelines name will be `some-user-wip-something`
and `some-user-wip-something-pr`.

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
    * Uploads artifacts to GCS
* Publishes to GitHub release (TODO)
* Detects changes to pipeline sources and auto updates

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
* Detects changes to pipeline sources and auto updates

# Details

This Concourse pipeline YAML is rendered using `ytt`. Depends on output from `git` and `gcloud`.

## Dependencies

* [Google Cloud](https://console.cloud.google.com)
* [Concourse](https://concourse-ci.org) v6.5.0+
* [`ytt`](https://get-ytt.io) v0.28.0+
* [`git`](https://git-scm.com) v2.25.2+
* [`yq`](https://github.com/mikefarah/yq) v4.6.0+ (optional for set-pipeline script)
* [`gcloud`](https://cloud.google.com/sdk/docs/install) SDK

## Layout

* base - Defines all common tasks across both pipelines.
* release - Defines tasks tasks for release pipeline only.
* pr - Defines tasks for pr pipeline only.
* lib - ytt functions used by all templates.
* docker/task - Minimal image required to communicate with builders.

# TODO

## Concourse Installation

* Resolve chicken/egg problem with external API address.
    * `helm install concourse concourse/concourse`
    * `helm upgrade concourse concourse/concourse --set web.service.api.type=LoadBalancer,concourse.web.externalUrl=http://1.2.3.4:8080`
* Task for getting secrets into k8s.
    * `kubectl create secret generic gcr-json-key --from-literal "value=$(cat XXX.json)" --namespace=concourse-main`
    * `kubectl create secret generic github-access-token --from-literal "value=XXX" --namespace=concourse-main`
    * `kubectl create secret generic github-private-key --from-literal "value=$(cat XXX)" --namespace=concourse-main`
* Use docker locally for initial pipeline deployment to avoid `gcloud`, `ytt`, and `fly` version issues.

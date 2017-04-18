/*
 * libcluster -- A collection of Bayesian clustering algorithms
 * Copyright (C) 2013  Daniel M. Steinberg (d.steinberg@acfr.usyd.edu.au)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBCLUSTER_H
#define LIBCLUSTER_H

#include <stdexcept>
#include <iostream>
#include <eigen3/Eigen/Dense>
#include <vector>
#ifdef WITH_OMP
    #include <omp.h>
    #pragma comment(linker, "-lgomp")
#endif
#include "distributions.h"


//
// Namespace Definitions
//

/*! \brief Namespace that contains implementations of Bayesian mixture model
 *         based algorithms for clustering.
 *
 *  This namespace provides various Bayesian mixture models that can be used
 *  for clustering data. The algorithms that have been implemented are:
 *
 *    - Variational Dirichlet Process (VDP) for Gaussian observations [1], see
 *      learnVDP().
 *    - The Bayesian Gaussian Mixture model [4] ch 11, see learnBGMM().
 *    - The Bayesian Gaussian Mixture model with diagonal covariance Gaussians,
 *      see learnDGMM().
 *    - Bayesian Exponential Mixture model with a Gamma prior, see learnBEMM().
 *    - Groups of Mixtures Clustering (GMC) model for Gaussian observations [3],
 *      see learnGMC().
 *    - Symmetric Groups of Mixtures Clustering (S-GMC) model for Gaussian
 *      observations [3], see learnSGMC().
 *    - Groups of Mixtures Clustering model for diagonal covariance Gaussian
 *      observations, see learnDGMC().
 *    - Groups of Mixtures Clustering model for Exponential observations, see
 *      learnEGMC().
 *
 *    - A myriad  of other algorithms are possible, but have not been enumerated
 *      in the interfaces here.
 *
 *  All of these algorithms infer the number of clusters present in the data.
 *
 * [1] K. Kurihara, M. Welling, and N. Vlassis, Accelerated variational
 *     Dirichlet process mixtures, Advances in Neural Information Processing
 *     Systems, vol. 19, p. 761, 2007.
 *
 * [2] Y. Teh, K. Kurihara, and M. Welling. Collapsed variational inference
 *     for HDP. Advances in Neural Information Processing Systems,
 *     20:1481–1488, 2008.
 *
 * [3] D. M. Steinberg, An Unsupervised Approach to Modelling Visual Data, PhD
 *     Thesis, 2013.
 *
 * [4] C. M. Bishop, Pattern Recognition and Machine Learning. Cambridge, UK:
 *     Springer Science+Business Media, 2006.
 *
 * \note The greedy cluster splitting heuristic is different from that presented
 *       in [1] in that it is much faster, but may not choose the "best" cluster
 *       to split first.
 *
 * \note The code is generic enough to allow new clustering algorithms to be
 *       implemented quickly, since all of the algorithms use templated
 *       distribution types.
 *
 * \author Daniel Steinberg
 *         Australian Centre for Field Robotics
 *         The University of Sydney
 *
 * \date   25/11/2012
 *
 * \todo Find a better way to parallelise the vanilla clustering algorithms.
 * \todo Make this library more generic so discrete distributions can be used?
 * \todo I think there may be a bug in the sparse group variants.
 * \todo Should probably get rid of all the vector copies in splitting functions
 *       and interface functions.
 */
namespace libcluster
{


//
// Namespace constants
//

const double       PRIORVAL   = 1.0;     //!< Default prior hyperparameter value
const unsigned int TRUNC      = 100;     //!< Truncation level for classes
const unsigned int SPLITITER  = 15;      //!< Max number of iter. for split VBEM
const double       CONVERGE   = 1e-5f;       //!< Convergence threshold
const double       FENGYDEL   = CONVERGE/10; //!< Allowance for +ve F.E. steps
const double       ZEROCUTOFF = 0.1f;    //!< Obs. count cut off sparse updates


//
// Convenience Typedefs
//

//! Vector of double matricies
typedef std::vector<Eigen::MatrixXd>                  vMatrixXd;

//! Vector of vectors of double matricies
typedef std::vector< std::vector<Eigen::MatrixXd> >   vvMatrixXd;


//
// Mixture Models for Clustering (cluster.cpp)
//

/*! \brief The learning algorithm for the Variational Dirichlet Process for
 *         Gaussian clusters.
 *
 * This function implements the VDP clustering algorithm as specified by [1],
 * however a different 'nesting' strategy is used. The nesting strategy sets all
 * q(z_n > K) = 0, rather than setting the parameter distributions equal to
 * their priors over this truncation bound, K. This is the same nesting strategy
 * as used in [2].
 *
 *  \param X the observation matrix, NxD where N is the number of observations,
 *         and D is the number of dimensions.
 *  \param qZ is an NxK matrix of the variational posterior approximation to
 *         p(Z|X). This will always be overwritten to start with one
 *         cluster.
 *  \param weights is the distributions over the mixture weights of the model.
 *  \param clusters is a vector of distributions over the cluster parameters
 *         of the model, this will be size K.
 *  \param clusterprior is the prior 'tuning' parameter for the cluster
 *         parameter distributions. This effects how many clusters will be
 *         found.
 *  \param verbose flag for triggering algorithm status messages. Default is
 *         0 = silent.
 *  \param nthreads sets the number of threads for the clustering algorithm to
 *         use. The group cluster algorithms take fuller advantage of this. The
 *         default value is automatically determined by OpenMP.
 *  \returns Final free energy
 *  \throws std::logic_error if there are invalid argument calls such as
 *          non-PSD matrix calculations.
 *  \throws std::runtime_error if there are runtime issues with the VDP
 *          algorithm such as negative free energy steps, unexpected empty
 *          clusters etc.
 */
double learnVDP (
    const Eigen::MatrixXd& X,
    Eigen::MatrixXd& qZ,
    distributions::StickBreak& weights,
    std::vector<distributions::GaussWish>& clusters,
    const double clusterprior = PRIORVAL,
    const bool verbose = false
#ifdef WITH_OMP
    , const unsigned int nthreads = omp_get_max_threads()
#endif
    );


/*! \brief The learning algorithm for a Bayesian Gaussian Mixture model.
 *
 * This function implements the Bayesian GMM clustering algorithm as specified
 * by [1]. In practice I have found this performs almost identically to the VDP,
 * especially for large data cases.
 *
 *  \param X the observation matrix, NxD where N is the number of observations,
 *         and D is the number of dimensions.
 *  \param qZ is an NxK matrix of the variational posterior approximation to
 *         p(Z|X). This will always be overwritten to start with one
 *         cluster.
 *  \param weights is the distributions over the mixture weights of the model.
 *  \param clusters is a vector of distributions over the cluster parameters
 *         of the model, this will be size K.
 *  \param clusterprior is the prior 'tuning' parameter for the cluster
 *         parameter distributions. This effects how many clusters will be
 *         found.
 *  \param verbose flag for triggering algorithm status messages. Default is
 *         0 = silent.
 *  \param nthreads sets the number of threads for the clustering algorithm to
 *         use. The group cluster algorithms take fuller advantage of this. The
 *         default value is automatically determined by OpenMP.
 *  \returns Final free energy
 *  \throws std::logic_error if there are invalid argument calls such as
 *          non-PSD matrix calculations.
 *  \throws std::runtime_error if there are runtime issues with the VDP
 *          algorithm such as negative free energy steps, unexpected empty
 *          clusters etc.
 */
double learnBGMM (
    const Eigen::MatrixXd& X,
    Eigen::MatrixXd& qZ,
    distributions::Dirichlet& weights,
    std::vector<distributions::GaussWish>& clusters,
    const double clusterprior = PRIORVAL,
    const bool verbose = false
#ifdef WITH_OMP
    , const unsigned int nthreads = omp_get_max_threads()
#endif
    );


/*! \brief The learning algorithm for a Bayesian Gaussian Mixture model with
 *         diagonal covariance matrices.
 *
 * This function implements the Bayesian GMM clustering algorithm as specified
 * by [1] but with diagonal covariance matrices, i.e. this is a Naive-Bayes
 * assumption.
 *
 *  \param X the observation matrix, NxD where N is the number of observations,
 *         and D is the number of dimensions.
 *  \param qZ is an NxK matrix of the variational posterior approximation to
 *         p(Z|X). This will always be overwritten to start with one
 *         cluster.
 *  \param weights is the distributions over the mixture weights of the model.
 *  \param clusters is a vector of distributions over the cluster parameters
 *         of the model, this will be size K.
 *  \param clusterprior is the prior 'tuning' parameter for the cluster
 *         parameter distributions. This effects how many clusters will be
 *         found.
 *  \param verbose flag for triggering algorithm status messages. Default is
 *         0 = silent.
 *  \param nthreads sets the number of threads for the clustering algorithm to
 *         use. The group cluster algorithms take fuller advantage of this. The
 *         default value is automatically determined by OpenMP.
 *  \returns Final free energy
 *  \throws std::logic_error if there are invalid argument calls such as
 *          negative diagonal covariance matrix calculations.
 *  \throws std::runtime_error if there are runtime issues with the VDP
 *          algorithm such as negative free energy steps, unexpected empty
 *          clusters etc.
 */
double learnDGMM (
    const Eigen::MatrixXd& X,
    Eigen::MatrixXd& qZ,
    distributions::Dirichlet& weights,
    std::vector<distributions::NormGamma>& clusters,
    const double clusterprior = PRIORVAL,
    const bool verbose = false
#ifdef WITH_OMP
    , const unsigned int nthreads = omp_get_max_threads()
#endif
    );


/*! \brief The learning algorithm for a Bayesian Exponential Mixture model.
 *
 * This function implements a Bayesian Exponential mixture model clustering
 * algorithm. The Exponential mixture model uses a Dirichlet prior on the
 * mixture weights, but an Exponential cluster distribution (with a Gamma
 * prior). Each dimension of the data is assumed independent i.e. this is a
 * Naive-Bayes assumption.
 *
 *  \param X the observation matrix, NxD where N is the number of observations,
 *         and D is the number of dimensions. X MUST be in the range [0, inf).
 *  \param qZ is an NxK matrix of the variational posterior approximation to
 *         p(Z|X). This will always be overwritten to start with one
 *         cluster.
 *  \param weights is the distributions over the mixture weights of the model.
 *  \param clusters is a vector of distributions over the cluster parameters
 *         of the model, this will be size K.
 *  \param clusterprior is the prior 'tuning' parameter for the cluster
 *         parameter distributions. This effects how many clusters will be
 *         found.
 *  \param verbose flag for triggering algorithm status messages. Default is
 *         0 = silent.
 *  \param nthreads sets the number of threads for the clustering algorithm to
 *         use. The group cluster algorithms take fuller advantage of this. The
 *         default value is automatically determined by OpenMP.
 *  \returns Final free energy
 *  \throws std::logic_error if there are invalid argument calls.
 *  \throws std::runtime_error if there are runtime issues with the VDP
 *          algorithm such as negative free energy steps, unexpected empty
 *          clusters etc.
 */
double learnBEMM (
    const Eigen::MatrixXd& X,
    Eigen::MatrixXd& qZ,
    distributions::Dirichlet& weights,
    std::vector<distributions::ExpGamma>& clusters,
    const double clusterprior = PRIORVAL,
    const bool verbose = false
#ifdef WITH_OMP
    , const unsigned int nthreads = omp_get_max_threads()
#endif
    );


/*! \brief The learning algorithm for the Groups of Mixtures Clustering model.
 *
 * This function implements the Groups of Mixtues Clustering model algorithm
 * as specified by [3], with the additional "sparse" option. The GMC uses a
 * Generalised Dirichlet prior on the group mixture weights and Gaussian cluster
 * distributions (With Gausian-Wishart priors). This algorithm is similar to a
 * one-level Hierarchical Dirichlet process with Gaussian observations.
 *
 *  \param X the observation matrices. Vector of N_jxD matrices where N_j is
 *         the number of observations in each group, j, and D is the number
 *         of dimensions.
 *  \param qZ is a vector of N_jxK matrices of the variational posterior
 *         approximations to p(z_j|X_j). K is the number of model clusters.
 *         This will always be overwritten to start with one cluster.
 *  \param weights is a vector of distributions over the mixture weights of the
 *         model, for each group of data, J.
 *  \param clusters is a vector of distributions over the cluster parameters
 *         of the model, this will be size K.
 *  \param clusterprior is the prior 'tuning' parameter for the cluster
 *         parameter distributions. This effects how many clusters will be
 *         found.
 *  \param sparse flag for enabling the "sparse" updates for the GMC. Some
 *         small amount of accuracy is traded off for a potentially large
 *         speed increase by not updating zero group weight cluster
 *         observation likelihoods. By default this is not enabled.
 *  \param verbose flag for triggering algorithm status messages. Default is
 *         0 = silent.
 *  \param nthreads sets the number of threads for the clustering algorithm to
 *         use. The group cluster algorithms take fuller advantage of this. The
 *         default value is automatically determined by OpenMP.
 *  \returns Final free energy
 *  \throws std::logic_error if there are invalid argument calls such as
 *          non-PSD matrix calculations.
 *  \throws std::runtime_error if there are runtime issues with the GMC
 *          algorithm such as negative free energy steps, unexpected empty
 *          clusters etc.
 */
double learnGMC (
    const vMatrixXd& X,
    vMatrixXd& qZ,
    std::vector<distributions::GDirichlet>& weights,
    std::vector<distributions::GaussWish>& clusters,
    const double clusterprior = PRIORVAL,
    const bool sparse = false,
    const bool verbose = false
#ifdef WITH_OMP
    , const unsigned int nthreads = omp_get_max_threads()
#endif
    );


/*! \brief The learning algorithm for the Symmetric Groups of Mixtures
 *         Clustering model.
 *
 * This function implements the Symmetric Groups of Mixtures Clustering model
 * as specified by [3], with the additional "sparse" option. The Symmetric GMC
 * uses a symmetric Dirichlet prior on the group mixture weights and Gaussian
 * cluster distributions (With Gausian-Wishart priors). This algorithm is
 * similar to latent Dirichlet allocation with Gaussian observations.
 *
 *  \param X the observation matrices. Vector of N_jxD matrices where N_j is
 *         the number of observations in each group, j, and D is the number
 *         of dimensions.
 *  \param qZ is a vector of N_jxK matrices of the variational posterior
 *         approximations to p(z_j|X_j). K is the number of model clusters.
 *         This will always be overwritten to start with one cluster.
 *  \param weights is a vector of distributions over the mixture weights of the
 *         model, for each group of data, J.
 *  \param clusters is a vector of distributions over the cluster parameters
 *         of the model, this will be size K.
 *  \param clusterprior is the prior 'tuning' parameter for the cluster
 *         parameter distributions. This effects how many clusters will be
 *         found.
 *  \param sparse flag for enabling the "sparse" updates for the GMC. Some
 *         small amount of accuracy is traded off for a potentially large
 *         speed increase by not updating zero group weight cluster
 *         observation likelihoods. By default this is not enabled.
 *  \param verbose flag for triggering algorithm status messages. Default is
 *         0 = silent.
 *  \param nthreads sets the number of threads for the clustering algorithm to
 *         use. The group cluster algorithms take fuller advantage of this. The
 *         default value is automatically determined by OpenMP.
 *  \returns Final free energy
 *  \throws std::logic_error if there are invalid argument calls such as
 *          non-PSD matrix calculations.
 *  \throws std::runtime_error if there are runtime issues with the GMC
 *          algorithm such as negative free energy steps, unexpected empty
 *          clusters etc.
 */
double learnSGMC (
    const vMatrixXd& X,
    vMatrixXd& qZ,
    std::vector<distributions::Dirichlet>& weights,
    std::vector<distributions::GaussWish>& clusters,
    const double clusterprior = PRIORVAL,
    const bool sparse = false,
    const bool verbose = false
#ifdef WITH_OMP
    , const unsigned int nthreads = omp_get_max_threads()
#endif
    );


/*! \brief The learning algorithm for the Groups of Mixtures Clustering model
 *         but with diagonal covariance Gaussians.
 *
 * This function implements the Groups of Mixtues Clustering model algorithm
 * as specified by [3], with the additional "sparse" option but with diagonal
 * covariance Gaussians, i.e. this is a Naive-Bayes assumption. The DGMC uses a
 * Generalised Dirichlet prior on the group mixture weights and Normal cluster
 * distributions (With Normal-Gamma priors). This algorithm is similar to a
 * one-level Hierarchical Dirichlet process with Gaussian observations.
 *
 *  \param X the observation matrices. Vector of N_jxD matrices where N_j is
 *         the number of observations in each group, j, and D is the number
 *         of dimensions.
 *  \param qZ is a vector of N_jxK matrices of the variational posterior
 *         approximations to p(z_j|X_j). K is the number of model clusters.
 *         This will always be overwritten to start with one cluster.
 *  \param weights is a vector of distributions over the mixture weights of the
 *         model, for each group of data, J.
 *  \param clusters is a vector of distributions over the cluster parameters
 *         of the model, this will be size K.
 *  \param clusterprior is the prior 'tuning' parameter for the cluster
 *         parameter distributions. This effects how many clusters will be
 *         found.
 *  \param sparse flag for enabling the "sparse" updates for the GMC. Some
 *         small amount of accuracy is traded off for a potentially large
 *         speed increase by not updating zero group weight cluster
 *         observation likelihoods. By default this is not enabled.
 *  \param verbose flag for triggering algorithm status messages. Default is
 *         0 = silent.
 *  \param nthreads sets the number of threads for the clustering algorithm to
 *         use. The group cluster algorithms take fuller advantage of this. The
 *         default value is automatically determined by OpenMP.
 *  \returns Final free energy
 *  \throws std::logic_error if there are invalid argument calls such as
 *          negative diagonal covariance matrix calculations.
 *  \throws std::runtime_error if there are runtime issues with the GMC
 *          algorithm such as negative free energy steps, unexpected empty
 *          clusters etc.
 */
double learnDGMC (
    const vMatrixXd& X,
    vMatrixXd& qZ,
    std::vector<distributions::GDirichlet>& weights,
    std::vector<distributions::NormGamma>& clusters,
    const double clusterprior = PRIORVAL,
    const bool sparse = false,
    const bool verbose = false
#ifdef WITH_OMP
    , const unsigned int nthreads = omp_get_max_threads()
#endif
    );


/*! \brief The learning algorithm for the Exponential Groups of Mixtures
 *         Clustering model.
 *
 * This function implements the Exponential Groups of Mixtures Clustering model,
 * with the additional "sparse" option. The Exponential GMC uses a Generalised
 * Dirichlet prior on the group mixture weights, but an Exponential cluster
 * distribution (with a Gamma prior). This algorithm is similar to a
 * one-level Hierarchical Dirichlet process with Exponential observations.
 *
 *  \param X the observation matrices. Vector of N_jxD matrices where N_j is
 *         the number of observations in each group, j, and D is the number
 *         of dimensions. X MUST be in the range [0, inf).
 *  \param qZ is a vector of N_jxK matrices of the variational posterior
 *         approximations to p(z_j|X_j). K is the number of model clusters.
 *         This will always be overwritten to start with one cluster.
 *  \param weights is a vector of distributions over the mixture weights of the
 *         model, for each group of data, J.
 *  \param clusters is a vector of distributions over the cluster parameters
 *         of the model, this will be size K.
 *  \param clusterprior is the prior 'tuning' parameter for the cluster
 *         parameter distributions. This effects how many clusters will be
 *         found.
 *  \param sparse flag for enabling the "sparse" updates for the GMC. Some
 *         small amount of accuracy is traded off for a potentially large
 *         speed increase by not updating zero group weight cluster
 *         observation likelihoods. By default this is not enabled.
 *  \param verbose flag for triggering algorithm status messages. Default is
 *         0 = silent.
 *  \param nthreads sets the number of threads for the clustering algorithm to
 *         use. The group cluster algorithms take fuller advantage of this. The
 *         default value is automatically determined by OpenMP.
 *  \returns Final free energy
 *  \throws std::logic_error if there are invalid argument calls.
 *  \throws std::runtime_error if there are runtime issues with the GMC
 *          algorithm such as negative free energy steps, unexpected empty
 *          clusters etc.
 */
double learnEGMC (
    const vMatrixXd& X,
    vMatrixXd& qZ,
    std::vector<distributions::GDirichlet>& weights,
    std::vector<distributions::ExpGamma>& clusters,
    const double clusterprior = PRIORVAL,
    const bool sparse = false,
    const bool verbose = false            
#ifdef WITH_OMP
    , const unsigned int nthreads = omp_get_max_threads()
#endif
    );

}
#endif // LIBCLUSTER_H

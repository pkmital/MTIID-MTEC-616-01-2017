/*
 
 © Parag K Mital, parag@pkmital.com
 
 The Software is and remains the property of Parag K Mital
 ("pkmital") The Licensee will ensure that the Copyright Notice set
 out above appears prominently wherever the Software is used.
 
 The Software is distributed under this Licence:
 
 - on a non-exclusive basis,
 
 - solely for non-commercial use in the hope that it will be useful,
 
 - "AS-IS" and in order for the benefit of its educational and research
 purposes, pkmital makes clear that no condition is made or to be
 implied, nor is any representation or warranty given or to be
 implied, as to (i) the quality, accuracy or reliability of the
 Software; (ii) the suitability of the Software for any particular
 use or for use under any specific conditions; and (iii) whether use
 of the Software will infringe third-party rights.
 
 pkmital disclaims:
 
 - all responsibility for the use which is made of the Software; and
 
 - any liability for the outcomes arising from using the Software.
 
 The Licensee may make public, results or data obtained from, dependent
 on or arising out of the use of the Software provided that any such
 publication includes a prominent statement identifying the Software as
 the source of the results or the data, including the Copyright Notice
 and stating that the Software has been made available for use by the
 Licensee under licence from pkmital and the Licensee provides a copy of
 any such publication to pkmital.
 
 The Licensee agrees to indemnify pkmital and hold them
 harmless from and against any and all claims, damages and liabilities
 asserted by third parties (including claims for negligence) which
 arise directly or indirectly from the use of the Software or any
 derivative of it or the sale of any products based on the
 Software. The Licensee undertakes to make no liability claim against
 any employee, student, agent or appointee of pkmital, in connection
 with this Licence or the Software.
 
 
 No part of the Software may be reproduced, modified, transmitted or
 transferred in any form or by any observations, electronic or mechanical,
 without the express permission of pkmital. pkmital's permission is not
 required if the said reproduction, modification, transmission or
 transference is done without financial return, the conditions of this
 Licence are imposed upon the receiver of the product, and all original
 and amended source code is included in any transmitted product. You
 may be held legally responsible for any copyright infringement that is
 caused or encouraged by your failure to abide by these terms and
 conditions.
 
 You are not permitted under this Licence to use this Software
 commercially. Use for which any financial return is received shall be
 defined as commercial use, and includes (1) integration of all or part
 of the source code or the Software into a product for sale or license
 by or on behalf of Licensee to third parties or (2) use of the
 Software or any derivative of it for research with the final aim of
 developing software products for sale or license to a third party or
 (3) use of the Software or any derivative of it for research with the
 final aim of developing non-software products for sale or license to a
 third party, or (4) use of the Software to provide any service to an
 external organisation for which payment is received. If you are
 interested in using the Software commercially, please contact pkmital to
 negotiate a licence. Contact details are: parag@pkmital.com
 
 */


#pragma once

#include <eigen3/Eigen/Dense>
#include "libcluster.h"
#include "distributions.h"
#include "pkmMatrix.h"

using Eigen::MatrixXd;
using Eigen::VectorXd;
using namespace std;
using namespace libcluster;
using namespace distributions;


class pkmDPGMM {
public:
    
    void allocate(int total_observations, int feature_dimensions)
    {
        num_features = feature_dimensions;
//        totalObservations = totalObservations == 0 ? 100 : 0;
//        data = MatrixXd(totalObservations, num_features);
        circular_mat = pkm::Mat(total_observations, num_features);
        circular_mat.resetCircularRowCounter();
        circular_mat.printAbbrev();
        data_ptr = (double *)malloc(sizeof(double) * total_observations * num_features);
        
        count_max = total_observations;
        count_current = 0;
    }
    
    void resize(int sz)
    {
        if (count_current > 0) {
            if (bFilled)
            {
                
            }
            else {
                
            }
        }
    }
    
    void addObservation(vector<float> observation)
    {
        circular_mat.insertRowCircularly(observation);
        
        count_current++;
        
        if(count_current > count_max)
            bFilled = true;
    }
    
    static void cluster(const pkm::Mat &input_data, pkm::Mat & cluster_means, pkm::Mat & cluster_covars, pkm::Mat & cluster_priors, size_t feature_length)
    {
        double *data_d_ptr = (double *)malloc(sizeof(double) * input_data.size());
        input_data.copyToDouble(data_d_ptr);
        
        MatrixXd data = Eigen::Map<MatrixXd>(data_d_ptr, input_data.rows * input_data.cols / feature_length, feature_length);
        if (input_data.rows == 0) {
            printf("[ERROR]::pkmDPGMM::cluster() data has 0 observations...\n");
            free(data_d_ptr);
            return;
        }
        
        MatrixXd                                    qZ;
        distributions::StickBreak                   weights;
        std::vector<distributions::GaussWish>       clusters;
        
//        clock_t start = clock();
        learnVDP(data, qZ, weights, clusters);
        
//        double stop = (double)((clock() - start))/CLOCKS_PER_SEC;
//        cout << "GMC Elapsed time = " << stop << " sec." << endl;
//        
//        cout << endl << "Cluster Weights:" << endl;
//        cout << weights.Elogweight().exp().transpose() << endl;
//
//        cout << endl << "Cluster observations:" << endl;
        for (vector<GaussWish>::iterator k=clusters.begin(); k < clusters.end(); ++k)
        {
            Eigen::RowVectorXd mean_i = k->getmean();
            pkm::Mat mean_i_m;
            mean_i_m.copyFromDouble(mean_i.data(), mean_i.rows(), mean_i.cols());
            cluster_means.push_back(mean_i_m);
            
            Eigen::MatrixXd cov_i = k->getcov();
            pkm::Mat cov_i_m;
            cov_i_m.copyFromDouble(cov_i.data(), cov_i.rows(), cov_i.cols());
            cluster_covars.push_back(cov_i_m);
            
            cluster_priors.push_back((float)k->getprior());
        }
        
        free(data_d_ptr);
    }
    
    void cluster()
    {
        cout << circular_mat.current_row << endl;
        if (!circular_mat.isCircularInsertionFull()) {
            return;
        }
        circular_mat.copyToDouble(data_ptr);
        circular_mat.printAbbrev();
        
        MatrixXd data = Eigen::Map<MatrixXd>(data_ptr, circular_mat.rows, circular_mat.cols);
        
        
        if (count_current == 0) {
            printf("[ERROR]::pkmDPGMM::cluster() Add observations using 'addMean(vector<double> observation)' first...\n");
            return;
        }
        
        MatrixXd                                    qZ;
        distributions::StickBreak                   weights;
        std::vector<distributions::GaussWish>       clusters;
        clock_t start = clock();
        if (bFilled)
            learnVDP(data, qZ, weights, clusters);
        else
            learnVDP(data.block(0, 0, count_current, num_features), qZ, weights, clusters);
        double stop = (double)((clock() - start))/CLOCKS_PER_SEC;
        cout << "GMC Elapsed time = " << stop << " sec." << endl;
        
        cout << endl << "Cluster Weights:" << endl;
        cout << weights.Elogweight().exp().transpose() << endl;
        
        cout << endl << "Cluster observations:" << endl;
        for (vector<GaussWish>::iterator k=clusters.begin(); k < clusters.end(); ++k)
            cout << k->getmean() << endl;
        
        cout << endl << "Cluster covariances:" << endl;
        for (vector<GaussWish>::iterator k=clusters.begin(); k < clusters.end(); ++k)
            cout << k->getcov() << endl << endl;
    }
    
private:
    
    // elements in the mixture
    int                                         count_current,
                                                count_max;

    // dimension of data
    int                                         num_features;
    
    // if the data has been filled to maximum
    bool                                        bFilled;
    
    // storage for observations
//    MatrixXd                                    data;
    // pkm Matrix storage for circular data
    pkm::Mat                                    circular_mat;
    double *                                    data_ptr;
    
    // storage for clustering results
    MatrixXd                                    qZ;
    distributions::StickBreak                   weights;
    std::vector<distributions::GaussWish>       clusters;
    
};

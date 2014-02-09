#include "photogram.h"
#include "haversine_dist.h"

_INITIALIZE_EASYLOGGINGPP

typedef struct {
    double  lat;
    double  lon;
} coords;

typedef HaversineDist<double> Haversine_64F;
typedef flann::L1<double> L1_64F;

static const coords my_world[] = {
    // up left
    { -122.5064446,  37.7186645 },
    // low right
    { -122.5051955,  37.7174765 },
    // center
    { -122.5058201,  37.7180705},

    { 0, 0}
};

// Mat result is num_features x feature dimensions (lat, lon)
void create_test_set(Mat &result) {

    Mat row = Mat::zeros(1, 2, CV_64F);
    for (int i = 0; my_world[i].lat != 0; i++) {
        row.at<double>(0,0) = my_world[i].lat;
        row.at<double>(0,1) = my_world[i].lon;
        result.push_back(row);
    }
}

// opencv prototype for knnsearch :

    // void knnSearch(const vector<ElementType>& query, vector<int>& indices,
    //                    vector<DistanceType>& dists, int knn, const ::cvflann::SearchParams& params);
    // void knnSearch(const Mat& queries, Mat& indices, Mat& dists, int knn, const ::cvflann::SearchParams& params);



int main() {
    Mat dataset;
    create_test_set(dataset);
    LinearIndexParams linear_params;
    cvflann::SearchParams search_params;

    LOG(DEBUG) << "Test coordinates: " << endl << dataset;

    // total number of queries q
    int queries = 2;

    Mat query = Mat::zeros(queries, 2, CV_64F);
    // query 1 is up right;
    query.at<double>(0,0) = -122.5051955;
    query.at<double>(0,1) = 37.7186645;

    // query 2 is center (like in the dataset)
    query.at<double>(1,0) = -122.5058201;
    query.at<double>(1,1) = 37.7180705;

    // find n neighbors
    int knn = 3;

    // q queries, n knn neighbors
    Mat neighbors_indices(queries, knn, CV_32S);
    // CV_64F since Haversine is 64F
    Mat neighbors_distances(queries, knn, CV_64F);

    // use Harverstine distance (better for a really big surface)
    // and linear params (brute force)
    flann::GenericIndex<Haversine_64F> flann = flann::GenericIndex<Haversine_64F>(dataset, linear_params);

    flann.knnSearch(query, neighbors_indices,  neighbors_distances, knn, search_params);


    LOG(DEBUG)
        << "HARVESTINE Distances: " << endl
        << "Neighbors indices: " << endl << neighbors_indices << endl
        << "Neighbors distances (in kilo meters): " << endl << neighbors_distances;


    Mat neighbors_indices_l1(queries, knn, CV_32S);
    Mat neighbors_distances_l1(queries, knn, CV_64F);

    // use L1 (Manhattan distance) for a rough approximation coords relative to each other
    // should work on a small surface
    // and 4 parallel kd-trees for storage
    KDTreeIndexParams kd_tree_params = KDTreeIndexParams(4);
    flann::GenericIndex<L1_64F> flann_l1 = flann::GenericIndex<L1_64F>(dataset, kd_tree_params);
    flann_l1.knnSearch(query, neighbors_indices_l1,  neighbors_distances_l1, knn, search_params);



    LOG(DEBUG)
        << "MANHATTAN Distances: " << endl
        << "Neighbors indices: " << endl << neighbors_indices_l1 << endl
        << "Neighbors distances (not a unit): " << endl << neighbors_distances_l1;
}

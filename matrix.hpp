#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <iostream>
#include <cstdlib>

#include <boost/mpi.hpp>

#define MAX_MATRIX_RND 10

class Matrix_2x2 {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
        ar & r1c1;  ar & r1c2;
        ar & r2c1;  ar & r1c2;
    }

    int r1c1; int r1c2;
    int r2c1; int r2c2;

public:
    void rnd(){
        r1c1 = std::rand() % MAX_MATRIX_RND; r1c2 = std::rand() % MAX_MATRIX_RND;
        r2c1 = std::rand() % MAX_MATRIX_RND; r2c2 = std::rand() % MAX_MATRIX_RND;
    }

    void print() {
        std::cout << " | " << r1c1 << " | " << r1c2 << " | " << std::endl;
        std::cout << " | " << r2c1 << " | " << r2c2 << " | " << std::endl;
    }

    Matrix_2x2 operator*(const Matrix_2x2 rhs) const{
        Matrix_2x2 return_matrix;
        return_matrix.r1c1 = r1c1 * rhs.r1c1 + r1c2 * rhs.r2c1;   return_matrix.r1c2 = r1c1 * rhs.r1c2 + r1c2 * rhs.r2c2;
        return_matrix.r2c1 = r2c1 * rhs.r1c1 + r2c2 * rhs.r2c1;   return_matrix.r2c2 = r2c1 * rhs.r1c2 + r2c2 * rhs.r2c2;
        return return_matrix;
    }


};
BOOST_IS_MPI_DATATYPE(Matrix_2x2);

#endif

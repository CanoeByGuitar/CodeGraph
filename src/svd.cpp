//
// Created by ChenhuiWang on 2024/4/22.

// Copyright (c) 2024 Tencent. All rights reserved.
//
#include <iostream>
#include <Eigen/Dense>

int main() {
    Eigen::MatrixXd A(3,3);
    A << 1, 2, 3,
        4, 5, 6,
        7, 8, 9;

    std::cout << "Matrix A:\n" << A << std::endl << std::endl;

    Eigen::JacobiSVD<Eigen::MatrixXd> svd(A, Eigen::ComputeThinU | Eigen::ComputeThinV);

    Eigen::MatrixXd U = svd.matrixU();
    Eigen::VectorXd singularValues = svd.singularValues();
    Eigen::MatrixXd V = svd.matrixV();

    std::cout << "Left singular vectors (U):\n" << U << std::endl << std::endl;
    std::cout << "Singular values:\n" << singularValues << std::endl << std::endl;
    std::cout << "Right singular vectors (V):\n" << V << std::endl << std::endl;

    Eigen::MatrixXd Sigma = U * singularValues.asDiagonal() * V.transpose();
    std::cout << "Reconstructed matrix:\n" << Sigma << std::endl;

    return 0;
}

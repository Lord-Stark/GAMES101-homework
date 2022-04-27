#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_model_matrix_z_rotation(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.

    float theta = rotation_angle / 180.0f * MY_PI;

    float cos_theta = std::cos(theta);
    float sin_theta = std::sin(theta);

    Eigen::Matrix4f M_rotation;
    M_rotation << cos_theta, -sin_theta, 0, 0,
        sin_theta, cos_theta, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;

    model = M_rotation * model;

    return model;
}

Eigen::Matrix4f get_model_matrix_rotation(Eigen::Vector3f axis, float angle)
{
    Eigen::Matrix3f model_Temp = Eigen::Matrix3f::Identity();

    float theta = angle / 180.0f * MY_PI;
    float cos_theta = std::cos(theta);
    float sin_theta = std::sin(theta);

    Eigen::Matrix3f N;
    N << 0, -axis[2], axis[1],
        axis[2], 0, -axis[0],
        -axis[1], axis[0], 0;

    model_Temp = cos_theta * model_Temp + (1 - cos_theta) * axis * axis.transpose() + sin_theta * N;

    Eigen::Matrix4f model;
    model << model_Temp(0, 0), model_Temp(0, 1), model_Temp(0, 2), 0,
        model_Temp(1, 0), model_Temp(1, 1), model_Temp(1, 2), 0,
        model_Temp(2, 0), model_Temp(2, 1), model_Temp(2, 2), 0,
        0, 0, 0, 1;

    return model;
}

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0],
        0, 1, 0, -eye_pos[1],
        0, 0, 1, -eye_pos[2],
        0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    /*
        vertical_filed_of_view = fov / 180.0 * PI
        theta = vertical_filed_of_view / 2.0
        tan theta = half_height / near
        aspect_ratio = width / height
    */

    float theta = eye_fov / 2.0f / 180.f * MY_PI;
    float height = zNear * std::tan(theta) * 2.0f;
    float width = height * aspect_ratio;

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.

    zNear = -zNear;
    zFar = -zFar;

    Eigen::Matrix4f P_persp2ortho;
    P_persp2ortho << zNear, 0, 0, 0,
        0, zNear, 0, 0,
        0, 0, zNear + zFar, -zNear * zFar,
        0, 0, 1, 0;

    Eigen::Matrix4f P_ortho_translate;
    P_ortho_translate << 1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, -(zNear + zFar) / 2.0f,
        0, 0, 0, 1;

    Eigen::Matrix4f P_ortho_scale;
    P_ortho_scale << 2.0f / width, 0, 0, 0,
        0, 2.0f / height, 0, 0,
        0, 0, 2.0f / (zNear - zFar), 0,
        0, 0, 0, 1;

    projection = P_ortho_scale * P_ortho_translate * P_persp2ortho * projection;

    return projection;
}

int main(int argc, const char **argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3)
    {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4)
        {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    // define an axis which passes through the origin
    Eigen::Vector3f axis{1, 1, 0};
    axis.normalize();

    std::cout << axis << std::endl;

    if (command_line)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix_rotation(axis, angle));
        // r.set_model(get_model_matrix_z_rotation(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix_rotation(axis, angle));
        // r.set_model(get_model_matrix_z_rotation(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        std::cout << angle << std::endl;

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a')
        {
            angle += 10;
        }
        else if (key == 'd')
        {
            angle -= 10;
        }
        std::cout << "Adjust angle to: " << angle << "." << std::endl;
    }

    return 0;
}

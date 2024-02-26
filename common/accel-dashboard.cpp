// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2024 Intel Corporation. All Rights Reserved.

#include "accel-dashboard.h"

using namespace rs2;
using namespace rsutils::string;

accel_dashboard::accel_dashboard( std::string name )
    : stream_dashboard( name, 30 )
    , last_time( glfwGetTime() )
    , x_value( 0 )
    , y_value( 0 )
    , z_value( 0 )
    , n_value( 0 )
    , frame_rate( MAX_FRAME_RATE )
{
    clear( true );
}

void accel_dashboard::draw( ux_window & win, rect r )
{
    if( accel_params[curr_accel_param_position] == "X" )
    {
        auto x_hist = read_shared_data< std::deque< float > >( [&]() { return x_history; } );
        for( int i = 0; i < x_hist.size(); i++ )
        {
            add_point( (float)i, (float)x_hist[i] );
        }
    }

    if( accel_params[curr_accel_param_position] == "Y" )
    {
        auto y_hist = read_shared_data< std::deque< float > >( [&]() { return y_history; } );
        for( int i = 0; i < y_hist.size(); i++ )
        {
            add_point( (float)i, (float)y_hist[i] );
        }
    }

    if( accel_params[curr_accel_param_position] == "Z" )
    {
        auto z_hist = read_shared_data< std::deque< float > >( [&]() { return z_history; } );
        for( int i = 0; i < z_hist.size(); i++ )
        {
            add_point( (float)i, (float)z_hist[i] );
        }
    }

    if( accel_params[curr_accel_param_position] == "N" )
    {
        auto n_hist = read_shared_data< std::deque< float > >( [&]() { return n_history; } );
        for( int i = 0; i < n_hist.size(); i++ )
        {
            add_point( (float)i, (float)n_hist[i] );
        }
    }

    r.h -= ImGui::GetTextLineHeightWithSpacing() + 10;
    draw_dashboard( win, r );

    ImGui::SetCursorPosX( ImGui::GetCursorPosX() + 40 );
    ImGui::SetCursorPosY( ImGui::GetCursorPosY() );

    show_radiobuttons();

    ImGui::SameLine();

    show_data_rate_slider();
}

int accel_dashboard::get_height() const
{
    return (int)( 160 + ImGui::GetTextLineHeightWithSpacing() );
}

void accel_dashboard::clear( bool full )
{
    write_shared_data(
        [&]()
        {
            if( full )
            {
                x_history.clear();
                for( int i = 0; i < DEQUE_SIZE; i++ )
                    x_history.push_back( 0 );

                y_history.clear();
                for( int i = 0; i < DEQUE_SIZE; i++ )
                    y_history.push_back( 0 );

                z_history.clear();
                for( int i = 0; i < DEQUE_SIZE; i++ )
                    z_history.push_back( 0 );

                n_history.clear();
                for( int i = 0; i < DEQUE_SIZE; i++ )
                    n_history.push_back( 0 );
            }
        } );
}

void accel_dashboard::process_frame( rs2::frame f )
{
    write_shared_data(
        [&]()
        {
            if( f && f.is< rs2::motion_frame >() )
            {
                double ts = glfwGetTime();
                auto it = frame_to_time.find( f.get_profile().unique_id() );

                if( ts - last_time > frame_rate && it != frame_to_time.end() )
                {
                    rs2::motion_frame accel_frame = f.as< rs2::motion_frame >();

                    x_value = accel_frame.get_motion_data().x;
                    y_value = accel_frame.get_motion_data().y;
                    z_value = accel_frame.get_motion_data().z;
                    n_value = std::sqrt( ( x_value * x_value ) + ( y_value * y_value ) + ( z_value * z_value ) );

                    if( x_history.size() > DEQUE_SIZE )
                        x_history.pop_front();
                    if( y_history.size() > DEQUE_SIZE )
                        y_history.pop_front();
                    if( z_history.size() > DEQUE_SIZE )
                        z_history.pop_front();
                    if( n_history.size() > DEQUE_SIZE )
                        n_history.pop_front();

                    x_history.push_back( x_value );
                    y_history.push_back( y_value );
                    z_history.push_back( z_value );
                    n_history.push_back( n_value );

                    last_time = ts;
                }

                frame_to_time[f.get_profile().unique_id()] = ts;
            }
        } );
}

void accel_dashboard::show_radiobuttons()
{
    ImGui::PushStyleColor( ImGuiCol_Text, from_rgba( 233, 0, 0, 255, true ) );
    ImGui::RadioButton( "X", &curr_accel_param_position, 0 );
    if( ImGui::IsItemHovered() )
        ImGui::SetTooltip( "%s", "Show accel X" );
    ImGui::PopStyleColor();
    ImGui::SameLine();

    ImGui::PushStyleColor( ImGuiCol_Text, from_rgba( 0, 255, 0, 255, true ) );
    ImGui::RadioButton( "Y", &curr_accel_param_position, 1 );
    if( ImGui::IsItemHovered() )
        ImGui::SetTooltip( "%s", "Show accel Y" );
    ImGui::PopStyleColor();
    ImGui::SameLine();

    ImGui::PushStyleColor( ImGuiCol_Text, from_rgba( 85, 89, 245, 255, true ) );
    ImGui::RadioButton( "Z", &curr_accel_param_position, 2 );
    if( ImGui::IsItemHovered() )
        ImGui::SetTooltip( "%s", "Show accel Z" );
    ImGui::PopStyleColor();
    ImGui::SameLine();

    ImGui::PushStyleColor( ImGuiCol_Text, from_rgba( 255, 255, 255, 255, true ) );
    ImGui::RadioButton( "N", &curr_accel_param_position, 3 );
    if( ImGui::IsItemHovered() )
        ImGui::SetTooltip( "%s", "Show Normal" );
    ImGui::PopStyleColor();
}

void accel_dashboard::show_data_rate_slider()
{
    ImGui::PushItemWidth( 100 );
    ImGui::SliderFloat( "##rate", &frame_rate, MIN_FRAME_RATE, MAX_FRAME_RATE, "%.2f" );
    ImGui::GetWindowWidth();

    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip( "%s", std::string( rsutils::string::from() 
                                             << "Frame rate " 
                                             << std::fixed 
                                             << std::setprecision(1) 
                                             << frame_rate * 1000 
                                             << " mSec"
                                            ).c_str() );
    }
}
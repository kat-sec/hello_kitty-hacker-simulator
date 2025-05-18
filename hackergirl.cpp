// Include libraries
#include "splashkit.h" // Splashkit library
#include <memory>    // For smart pointers
#include <fstream>   // For file handling
#include <algorithm> // For std::find 
using std::to_string;

// Smart pointers for font and bitmap
std::shared_ptr<font> game_font_ptr;
std::shared_ptr<bitmap> hello_kitty_image_ptr;

// Load the game font, fallback to default if missing
font &game_font()
{
    if (!game_font_ptr)
    {
        try
        {
            game_font_ptr = std::make_shared<font>(load_font("arial", "arial.ttf"));
        }
        catch (const std::exception &e)
        {
            write_line("Error: Failed to load Arial font. Using default font.");
            game_font_ptr = std::make_shared<font>(load_font("default", "default.ttf"));
        }
    }
    return *game_font_ptr;
}

// Load Hello Kitty image using a shared pointer
void load_hello_kitty_image()
{
    if (!hello_kitty_image_ptr)
    {
        hello_kitty_image_ptr = std::make_shared<bitmap>(load_bitmap("hello_kitty", "hellokitty_png.png"));
    }
}

// Global variables
struct IPScanner
{
    vector<string> ip_addresses;   // List of IP addresses
    bool scan_in_progress = false; // Whether the scan is ongoing
};

IPScanner ip_scanner;

vector<string> ip_addresses;

// Structs for various variables

struct BruteForce
{
    string target_password = ""; // Target password will be set dynamically based on user input
    string current_guess = "";
    bool password_cracked = false;
};

struct Progress
{
    double value = 0;
    bool in_progress = false;
};

struct UIState
{
    bool is_button_hovered = false;         // Whether a button is hovered
    bool is_restart_button_hovered = false; // Whether the restart button is hovered
    string input_password = "";             // Stores password input by the user
    string input_username = "";             // Stores username input by the user
    double spinner_angle = 0;               // Spinner rotation angle
};

BruteForce brute_force;
Progress progress;
UIState ui_state;

// Track input mode (username or password)
bool is_entering_username = true;

// Structure for heart shapes
struct Heart
{
    double x, y;       // Position of the heart
    double size;       // Size of the heart
    double dy;         // Vertical velocity of the heart
    color heart_color; // Color of the heart
};

// Store active hearts
std::vector<Heart> hearts;

// Generate new hearts across the screen
void generate_hearts()
{
    while (hearts.size() < 10)
    {
        Heart heart;
        heart.x = rnd(0, screen_width());             // Random horizontal position
        heart.y = rnd(0, screen_height());            // Random vertical position
        heart.size = rnd(5, 10);                      // Size of hearts
        heart.dy = rnd(1, 3);                         // Random falling speed
        heart.heart_color = rgb_color(255, 102, 102); // Pale red color
        hearts.push_back(heart);
    }
}

// Draw a heart shape
void draw_heart(double x, double y, double size, color heart_color)
{
    const int points = 100;
    double scale = size / 12.0;

    vector<point_2d> heart_points; // Use SplashKit's point_2d type

    for (int i = 0; i < points; i++)
    {
        double t = (2 * M_PI * i) / points;
        // Parametric heart shape equations
        double px = 16 * pow(sin(t), 3);
        double py = 13 * cos(t) - 5 * cos(2 * t) - 2 * cos(3 * t) - cos(4 * t);

        px *= scale;
        py *= -scale;

        heart_points.push_back({x + px, y + py});
    }

    // Draw the heart as a filled triangle
    for (size_t i = 1; i < heart_points.size() - 1; ++i)
    {
        fill_triangle(heart_color, heart_points[0].x, heart_points[0].y, heart_points[i].x, heart_points[i].y, heart_points[i + 1].x, heart_points[i + 1].y);
    }
}

// Update and draw heart shapes
void update_and_draw_hearts()
{
    for (auto &heart : hearts)
    {
        heart.y += heart.dy; // Move heart downward

        // Reset heart if it moves off the bottom of the screen
        if (heart.y > screen_height())
        {
            heart.x = rnd(0, screen_width());             // Reset to a new random horizontal position
            heart.y = 0;                                  // Reset to the top of the screen
            heart.size = rnd(5, 8);                       // Size range for smaller hearts
            heart.dy = rnd(1, 3);                         // Reset falling speed
            heart.heart_color = rgb_color(255, 102, 102); // Reset color to custom pale red
        }

        // Draw the heart
        draw_heart(heart.x, heart.y, heart.size, heart.heart_color);
    }
}

// Draw a button
void draw_button(int x, int y, string text, bool hovered)
{
    if (hovered)
        fill_rectangle(COLOR_LIGHT_PINK, x, y, 200, 50); // Light pink when hovered

    else
        fill_rectangle(COLOR_PINK, x, y, 200, 50); // Pink when not hovered
    draw_rectangle(COLOR_MEDIUM_VIOLET_RED, x, y, 200, 50);

    draw_text(text, COLOR_DEEP_PINK, game_font(), 20, x + (200 - text_width(text, game_font(), 20)) / 2, y + 15);
}

// Check if the mouse is over the button
bool is_mouse_over_button(int x, int y)
{
    return mouse_x() >= x && mouse_x() <= x + 200 && mouse_y() >= y && mouse_y() <= y + 50;
}

// Generate random IP addresses
string generate_ip_addresses()
{
    return to_string(rnd(1, 255)) + "." +
           to_string(rnd(1, 255)) + "." +
           to_string(rnd(1, 255)) + "." +
           to_string(rnd(1, 255));
}

// IP address generation logic
void generate_and_store_ip_addresses(IPScanner &ip_scanner)
{
    if (ip_scanner.scan_in_progress && rnd(0, 30) == 0)
    {
        ip_scanner.ip_addresses.push_back(generate_ip_addresses());
    }
}

// Draw IP addresses
void draw_ip_addresses(const IPScanner &ip_scanner)
{
    // Hide "IP Addresses nearby:" text until scan is started
    if (!ip_scanner.ip_addresses.empty())
    {
        draw_text("IP Addresses nearby:", COLOR_BLACK, "Arial", 30, 590, 100);
        int y_offset = 130;
        for (const string &ip : ip_scanner.ip_addresses)
        {
            draw_text(ip, COLOR_BLACK, "Arial", 20, 590, y_offset);
            y_offset += 20;
        }
    }
}

// Draw a rotating spinner
void draw_spinner(int x, int y, double angle)
{
    for (int i = 0; i < 8; i++)
    {
        double segment_angle = angle + (i * (M_PI / 4));
        double segment_length = 20;

        double x_end = x + cos(segment_angle) * segment_length;
        double y_end = y + sin(segment_angle) * segment_length;

        draw_line(COLOR_HOT_PINK, x, y, x_end, y_end);
    }
}

// Generate brute-force effect
string generate_brute_force(int length)
{
    string charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*";

    string guess = "";
    for (int i = 0; i < length; i++)
    {
        guess += charset[rnd(0, charset.size() - 1)];
    }
    return guess;
}

// Read potential passwords from a file
std::vector<std::string> read_passwords_from_file(const std::string &filename)
{
    std::vector<std::string> passwords;
    std::ifstream inFile(filename);
    if (inFile)
    {
        std::string password;
        while (std::getline(inFile, password))
        {
            passwords.push_back(password);
        }
    }
    else
    {
        write_line("Error: Unable to open file for reading passwords.");
    }
    return passwords;
}

// Forward declaration for encrypt_password
string encrypt_password(const string &password, int shift);

// Brute-force function using passwords from the 'details' file
void simulate_brute_force(BruteForce &brute_force)
{
    if (!brute_force.target_password.empty())
    {
        // Use static variables to maintain state between function calls
        static std::vector<std::string> password_list;
        static size_t current_index = 0;
        static bool file_exhausted = false;
        static int brute_force_attempts = 0;
        static string last_target = "";       // Store the last target password to detect changes
        static size_t last_target_length = 0; // Track the previous target length for better reset control

        // Always reset if the target password has changed
        if (last_target != brute_force.target_password ||
            last_target_length != brute_force.target_password.length() ||
            brute_force.current_guess.empty())
        {

            // Reset all state variables
            password_list = read_passwords_from_file("details");
            current_index = 0;
            file_exhausted = false;
            brute_force_attempts = 0;
            brute_force.current_guess = "";                            // Force empty guess for proper initialisation
            last_target = brute_force.target_password;                 // Update last target
            last_target_length = brute_force.target_password.length(); // Update length tracking
        }

        if (!file_exhausted)
        {
            // Check passwords from the file
            for (int step = 0; step < 1000 && current_index < password_list.size(); step++, current_index++)
            {
                brute_force.current_guess = password_list[current_index];
                if (brute_force.current_guess == brute_force.target_password && !brute_force.password_cracked)
                {
                    brute_force.password_cracked = true;
                }
            }

            // If all passwords are checked and no match is found
            if (current_index >= password_list.size())
            {
                file_exhausted = true;
            }
        }

        if (file_exhausted)
        {
            // Start with one character, increase length as needed
            const string charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*";
            const size_t charset_size = charset.size();

            // Initialise with single character for systematic brute force
            if (brute_force.current_guess.empty())
            {
                brute_force.current_guess = string(1, charset[0]);

            } // Skip ahead if current guess length is already greater than target
            if (brute_force.current_guess.length() > brute_force.target_password.length())
            {
                // If we've somehow gotten a longer guess than target, reset to length 1
                brute_force.current_guess = string(1, charset[0]);
            }

            // Reduce guesses per frame to make the animation more visible
            int guesses_per_frame = 500;
            for (int step = 0; step < guesses_per_frame; step++)
            {
                // Check if we've found the password
                if (brute_force.current_guess == brute_force.target_password && !brute_force.password_cracked)
                {
                    brute_force.password_cracked = true;
                    std::ofstream outFile("details", std::ios::app);
                    if (outFile)
                    {
                        outFile << brute_force.target_password << std::endl;
                    }
                    // Don't return - continue the animation even after cracking
                }

                // Skip trying longer combinations than target length for efficiency
                if (brute_force.current_guess.length() > brute_force.target_password.length())
                {
                    brute_force.current_guess = string(1, charset[0]);
                    continue;
                }

                // Increment guess 
                size_t i = brute_force.current_guess.size() - 1;
                while (true)
                {
                    size_t pos = charset.find(brute_force.current_guess[i]);
                    if (pos < charset_size - 1)
                    {
                        brute_force.current_guess[i] = charset[pos + 1];
                        break;
                    }
                    else
                    {
                        brute_force.current_guess[i] = charset[0];

                        if (i == 0)
                        {
                            size_t new_length = brute_force.current_guess.size() + 1;
                            // Only increase length if we haven't exceeded target length
                            if (new_length <= brute_force.target_password.length())
                            {
                                brute_force.current_guess = string(new_length, charset[0]);
                            }
                            else
                            {
                                // If we would exceed target length, restart from length 1
                                brute_force.current_guess = string(1, charset[0]);
                            }
                            break;
                        }
                        i--;
                    }
                }
            }
        }
    } // Only show the testing animation if the password is not yet cracked
    if (!brute_force.password_cracked)
    {
        // Display the current brute force attempt with visual effect
        draw_text("Testing: " + brute_force.current_guess, COLOR_BLACK, "Arial", 20, 300, 500);

        // Show status information
        if (!brute_force.current_guess.empty())
        {
            string status = "Length: " + std::to_string(brute_force.current_guess.length()) +
                            " | Trying all combinations...";
            draw_text(status, COLOR_MEDIUM_VIOLET_RED, "Arial", 16, 300, 530);
        }
    }
    // If password is cracked inside this function, show success message
    if (brute_force.password_cracked && !brute_force.current_guess.empty())
    {
        // Apply Caesar cipher with shift of 3
        string encrypted_password = encrypt_password(brute_force.target_password, 3);

        // Display success message with more prominent colors 
        draw_text("SUCCESS! Password Cracked: " + brute_force.target_password, COLOR_HOT_PINK, "Arial", 24, 300, 480);

        // Make the encrypted password more visible 
        draw_text("Caesar Cipher (Shift 3): " + encrypted_password, COLOR_MEDIUM_VIOLET_RED, "Arial", 24, 300, 510);
    }
}

// Draw username input box
void draw_username_input(const string &username, bool is_active)
{
    draw_text("Enter Username: ", COLOR_BLACK, "Arial", 20, 150, 120);
    fill_rectangle(is_active ? COLOR_LIGHT_PINK : COLOR_WHITE, 300, 120, 200, 30);
    draw_text(username, COLOR_BLACK, "Arial", 20, 305, 125);
}

// Draw the password input box
void draw_password_input(const string &input_password, bool is_active)
{
    draw_text("Enter Password: ", COLOR_BLACK, "Arial", 20, 150, 170);
    fill_rectangle(is_active ? COLOR_LIGHT_PINK : COLOR_WHITE, 300, 170, 200, 30);

    // Mask password input with asterisks
    string masked_password = string(input_password.length(), '*');
    draw_text(masked_password, COLOR_BLACK, "Arial", 20, 305, 175);
}

// Reset all variables when restarting
void reset_game()
{
    progress.value = 0;                   // Reset progress bar
    brute_force.current_guess = "";       // Reset the guess for brute force
    brute_force.password_cracked = false; // Reset password crack status
    brute_force.target_password = "";     // Reset target password
    ip_scanner.ip_addresses.clear();      // Clear the IP addresses list
    progress.in_progress = false;         // Stop scanning
    ui_state.input_password = "";         // Clear password input
    ui_state.input_username = "";         // Clear username input
    is_entering_username = true;          // Reset to username input mode
}

// Draw the progress bar
void draw_progress_bar(Progress &progress)
{
    if (progress.value < 1)
    {
        fill_rectangle(COLOR_PINK, 300, 400, 200, 30);
        fill_rectangle(COLOR_HOT_PINK, 300, 400, progress.value * 200, 30);
        draw_rectangle(COLOR_MEDIUM_VIOLET_RED, 300, 400, 200, 30);
    }
}

// Encrypt a string using a Caesar cipher
string encrypt_password(const string &password, int shift)
{
    string encrypted = "";
    for (char c : password)
    {
        if (isalpha(c))
        {
            char base = isupper(c) ? 'A' : 'a';
            encrypted += (c - base + shift) % 26 + base;
        }
        else
        {
            encrypted += c;
        }
    }
    return encrypted;
}

// Save username and password to a file
void save_user_data(const std::string &filename)
{
    std::ofstream outFile(filename);
    if (outFile)
    {
        outFile << ui_state.input_username << std::endl;
        outFile << ui_state.input_password << std::endl;
    }
    else
    {
        write_line("Error: Unable to open file for saving user data.");
    }
}

// Load username and password from a file
void load_user_data(const std::string &filename)
{
    std::ifstream inFile(filename);
    if (inFile)
    {
        std::getline(inFile, ui_state.input_username);
        std::getline(inFile, ui_state.input_password);
    }
    else
    {
        write_line("Error: Unable to open file for loading user data.");
    }
}

// Save IP addresses to a file
void save_ip_addresses(const std::string &filename, const IPScanner &ip_scanner)
{
    std::ofstream outFile(filename);
    if (outFile)
    {
        for (const auto &ip : ip_scanner.ip_addresses)
        {
            outFile << ip << std::endl;
        }
    }
    else
    {
        write_line("Error: Unable to open file for saving IP addresses.");
    }
}

// Load IP addresses from a file
void load_ip_addresses(const std::string &filename, IPScanner &ip_scanner)
{
    std::ifstream inFile(filename);
    if (inFile)
    {
        ip_scanner.ip_addresses.clear();
        std::string ip;
        while (std::getline(inFile, ip))
        {
            ip_scanner.ip_addresses.push_back(ip);
        }
    }
    else
    {
        write_line("Note: No IP addresses file found or unable to open.");
    }
}

// Draw UI elements
void draw_ui(const string &title_text, UIState &ui_state, Progress &progress, BruteForce &brute_force, IPScanner &ip_scanner)
{

    // Clear the screen with a misty rose color
    clear_screen(COLOR_MISTY_ROSE);

    // Draw the Hello Kitty image
    draw_bitmap(*hello_kitty_image_ptr, 50, 300); 

    // Draw title
    draw_text(title_text, COLOR_HOT_PINK, game_font(), 30, 150, 50); 

    // Draw buttons
    ui_state.is_button_hovered = is_mouse_over_button(300, 250); 
    draw_button(300, 250, "Start/Stop Scan", ui_state.is_button_hovered);

    ui_state.is_restart_button_hovered = is_mouse_over_button(300, 320); 
    draw_button(300, 320, "Restart", ui_state.is_restart_button_hovered);

    // Display scanning progress
    if (progress.in_progress && progress.value < 1)
    {
        draw_text("Scanning...", COLOR_HOT_PINK, game_font(), 20, 300, 400); 
        draw_spinner(530, 410, ui_state.spinner_angle);                     

        // IP generation frequency
        generate_and_store_ip_addresses(ip_scanner);

        progress.value += 0.0025;
        if (progress.value >= 1)
        {
            progress.value = 1;
            draw_text("Scan Complete!", COLOR_HOT_PINK, game_font(), 30, 300, 400);

            // Stop the spinner when the scan is complete
            ui_state.spinner_angle = 0;
        }
    }
    else if (progress.value >= 1)
    {
        draw_text("Scan Complete!", COLOR_HOT_PINK, game_font(), 30, 300, 400);
    }
    else
    {
        draw_text("Scan Stopped", COLOR_HOT_PINK, game_font(), 20, 300, 400);
    }

    // Progress bar
    draw_progress_bar(progress);

    // Draw the collected IP addresses
    draw_ip_addresses(ip_scanner);

    // Draw username input box
    draw_username_input(ui_state.input_username, is_entering_username);

    // Draw the password input box
    draw_password_input(ui_state.input_password, !is_entering_username);

    // Generate hearts around Hello Kitty
    generate_hearts();

    // Update and draw hearts
    update_and_draw_hearts();
}

// Open the window and load assets
int main()
{
    sound_effect snd_effect;
    snd_effect = load_sound_effect("music", "music.mp3");

    // Play the music in a loop
    play_sound_effect(snd_effect, -1); // -1 indicates infinite looping

    // Open the window and load assets
    open_window("Hacker Girl Dashboard", 800, 600);

    load_hello_kitty_image();

    // Load user data at the start of the program
    load_user_data("details");

    // Load IP addresses at the start of the program
    load_ip_addresses("ip_addresses", ip_scanner);

    string title_text = "Welcome to your Hacker Dashboard";

    // Main game loop
    while (!quit_requested())
    {
        process_events();

        // Ensure the music plays in a continuous loop
        if (!sound_effect_playing(snd_effect))
        {
            play_sound_effect(snd_effect, 1, -1); // Play in a loop (-1 for infinite loops)
        }

        // Handle Start/Stop button click for scan
        if (ui_state.is_button_hovered && mouse_clicked(LEFT_BUTTON))
        {
            progress.in_progress = !progress.in_progress;
            ip_scanner.scan_in_progress = progress.in_progress;
        }

        // Handle Restart button click
        if (ui_state.is_restart_button_hovered && mouse_clicked(LEFT_BUTTON))
        {
            reset_game();
        }

        // Handle keypress for username and password input
        if (any_key_pressed())
        {
            if (key_typed(BACKSPACE_KEY))
            {
                // Handle backspace for the active input field
                if (is_entering_username && !ui_state.input_username.empty())
                {
                    ui_state.input_username.pop_back();
                }
                else if (!is_entering_username && !ui_state.input_password.empty())
                {
                    ui_state.input_password.pop_back();
                    // Always update the target password when it's modified
                    // This ensures brute force will start fresh with the new password
                    brute_force.target_password = ui_state.input_password;
                    brute_force.current_guess = "";
                    brute_force.password_cracked = false;
                }
            }
            else if (key_typed(TAB_KEY))
            {
                // Switch between username and password input
                is_entering_username = !is_entering_username;
            }
            else if (!is_entering_username && key_typed(RETURN_KEY))
            {
                // Finalise password input and start brute force
                if (!ui_state.input_password.empty())
                {
                    // Only reset if password changed
                    if (brute_force.target_password != ui_state.input_password)
                    {
                        brute_force.target_password = ui_state.input_password;
                        brute_force.current_guess = "";       // Reset current guess to empty to force restart
                        brute_force.password_cracked = false; // Reset cracked status
                    }
                }
            }
            else
            {
                // Alphabetical characters, symbols and numbers for user input
                for (char c = 32; c <= 126; c++)
                {
                    if (key_typed(static_cast<key_code>(c)))
                    {
                        if (is_entering_username)
                        {
                            ui_state.input_username += c;
                        }
                        else
                        {
                            ui_state.input_password += c;
                            // Always update the target password when characters are added
                            // This forces a complete reset of the brute force algorithm
                            brute_force.target_password = ui_state.input_password;
                            brute_force.current_guess = "";       // Reset current guess
                            brute_force.password_cracked = false; // Reset cracked status
                        }
                        break;
                    }
                }
            }
        }

        // Increment spinner angle for continuous rotation
        ui_state.spinner_angle += 0.1;
        if (ui_state.spinner_angle >= 2 * M_PI)
        {
            ui_state.spinner_angle -= 2 * M_PI;
        }

        // Draw the UI
        draw_ui(title_text, ui_state, progress, brute_force, ip_scanner);
        // Simulate brute force if a target password is set and not yet cracked
        if (!brute_force.target_password.empty() && !brute_force.password_cracked)
        {
            simulate_brute_force(brute_force);
        }
        else if (!brute_force.target_password.empty() && brute_force.password_cracked)
        {
            // Keep displaying the success and encryption message,
            // but don't continue testing password combinations
            string encrypted_password = encrypt_password(brute_force.target_password, 3);

            // Display success message with more prominent colors 
            draw_text("SUCCESS! Password Cracked: " + brute_force.target_password, COLOR_HOT_PINK, "Arial", 24, 300, 480);

            // Make the encrypted password more visible 
            draw_text("Caesar Cipher (Shift 3): " + encrypted_password, COLOR_MEDIUM_VIOLET_RED, "Arial", 24, 300, 510);
        }

        refresh_screen(60);
    }

    // Save user data before exiting the program
    save_user_data("details");

    // Save IP addresses before exiting the program
    save_ip_addresses("ip_addresses", ip_scanner);

    return 0;
}

import java.io.BufferedReader;
import java.io.FileReader;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;

public class Fixed {
    public static void sqlInjection(Connection conn, String name) throws Exception {
        String query = "SELECT * FROM users WHERE name = ?";
        try (PreparedStatement ps = conn.prepareStatement(query)) {
            ps.setString(1, name);
            try (ResultSet rs = ps.executeQuery()) {
                while (rs.next()) {
                    System.out.println(rs.getString("name"));
                }
            }
        }
    }

    public static void hardcodedPassword() {
        String password = System.getenv("DB_PASSWORD");
        if (password == null || password.isEmpty()) {
            System.out.println("DB_PASSWORD is not configured");
            return;
        }
        System.out.println("password loaded from environment");
    }

    public static void nullPointer() {
        String text = System.getenv("SAMPLE_TEXT");
        if (text != null) {
            System.out.println(text.length());
        } else {
            System.out.println("SAMPLE_TEXT is not configured");
        }
    }

    public static void resourceLeak() throws Exception {
        try (BufferedReader reader = new BufferedReader(new FileReader("input.txt"))) {
            System.out.println(reader.readLine());
        }
    }

    public static void main(String[] args) throws Exception {
        System.out.println("Fixed demo");
        hardcodedPassword();
        nullPointer();
        resourceLeak();
    }
}

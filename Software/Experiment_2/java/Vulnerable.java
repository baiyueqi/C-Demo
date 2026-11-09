import java.io.BufferedReader;
import java.io.FileReader;
import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.Statement;

public class Vulnerable {
    private static final String DB_PASSWORD = "P@ssw0rd";

    public static void sqlInjection(Connection conn, String name) throws Exception {
        String query = "SELECT * FROM users WHERE name = '" + name + "'";
        try (Statement st = conn.createStatement();
             ResultSet rs = st.executeQuery(query)) {
            while (rs.next()) {
                System.out.println(rs.getString("name"));
            }
        }
    }

    public static void hardcodedPassword() {
        System.out.println("database password: " + DB_PASSWORD);
    }

    public static void nullPointer() {
        String text = null;
        System.out.println(text.length());
    }

    public static void resourceLeak() throws Exception {
        BufferedReader reader = new BufferedReader(new FileReader("input.txt"));
        System.out.println(reader.readLine());
    }

    public static void main(String[] args) throws Exception {
        System.out.println("Vulnerable demo: pass 'run' to trigger vulnerable code paths.");
        if (args.length > 0 && "run".equals(args[0])) {
            hardcodedPassword();
            resourceLeak();
            nullPointer();
        }
    }
}

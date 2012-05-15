/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package filesstatistics.core;

import java.io.FileInputStream;
import java.io.IOException;
import java.util.Properties;

/**
 *
 * @author Rafal
 */
public class PropertiesReader {
    public static final String PROPERTIES_FILE_PATH = "./resources/statistics.properties";
    public static final String STATISTICS_FILE_PATH_KEY = "file.path";
    public static final String HEADERS_LIST_KEY = "headers";
    public static final String OPERATIONS_LIST_KEY = "operations";
    public static final String DATE_FORMAT = "dateFormat";
    
    private static Properties properties;
    
    public static Properties readProperties() throws IOException {
        System.out.println("Working Directory = " +
           System.getProperty("user.dir"));
        if(properties == null) {
            properties = new Properties();
            properties.load(new FileInputStream(PROPERTIES_FILE_PATH));
        }
        return properties;
    }
}

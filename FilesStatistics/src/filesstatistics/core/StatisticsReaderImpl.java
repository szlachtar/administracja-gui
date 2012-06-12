/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package filesstatistics.core;

import java.io.File;
import java.io.IOException;
import java.util.*;

/**
 *
 * @author Rafal
 */
public class StatisticsReaderImpl implements StatisticsReader {
    
    private Properties applicationProperties = null;
    
    public StatisticsReaderImpl() throws IOException {
        applicationProperties = PropertiesReader.readProperties();
    }

    @Override
    public Map<String, Map<String, Object>> readStatistics() throws IOException {
        
        Scanner scanner = new Scanner(new File((String)applicationProperties.get(PropertiesReader.STATISTICS_FILE_PATH_KEY)));
        
        Map<String, Map<String, Object>> statistics = new HashMap<String, Map<String,Object>>();
        
        int lineNum = 0;
        while(scanner.hasNextLine()) {
            lineNum++;
            DataStructure ds = parseLine(scanner.nextLine(), lineNum);
            if (ds == null) {
                System.out.println("continue");
                continue;
            }
            if(!statistics.containsKey(ds.getFileName())) {
                statistics.put(ds.getFileName(), new HashMap<String, Object>());
            }
            Map<String, Object> descriptionMap = statistics.get(ds.getFileName());
            if(!descriptionMap.containsKey(DATA_STRUCTURE_LIST_KEY)) {
                descriptionMap.put(DATA_STRUCTURE_LIST_KEY, new ArrayList<DataStructure>());
                descriptionMap.put(FILE_TYPE_KEY, ds.getFileType());
            }
            
            List<DataStructure> dsList = (List<DataStructure>)descriptionMap.get(DATA_STRUCTURE_LIST_KEY);
            dsList.add(ds);
            
            if(descriptionMap.containsKey(ds.getOperation())) {
                int count = (Integer)descriptionMap.get(ds.getOperation());
                count++;
                descriptionMap.put(ds.getOperation(), count);
            } else {
                OPERATIONS.add(ds.getOperation());
                descriptionMap.put(ds.getOperation(), 1);
            }
        }
        
        return statistics;
    }
    
    private DataStructure parseLine(String line, int lineNum) {
        DataStructure ds = null;
        try {
            String[] fields = line.split(" ");
            ds = new DataStructure();
            ds.setDate(Long.parseLong(fields[fields.length-1]));
            ds.setOperation(fields[fields.length-2]);
            ds.setFileType(fields[fields.length-3]);
            String s = fields[0];
            for(int i=1; i<fields.length-3; i++) {
                s += " " + fields[i];
            }
            ds.setFileName(s);
        } catch (Exception e) {
            System.err.println("Could not parse " + lineNum + " line: " + line);
            System.err.println("Due to exception: " + e.toString());
            ds = null;
        }
        return ds;
    }
}

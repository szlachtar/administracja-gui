/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package filesstatistics.charts;

import filesstatistics.core.DataStructure;
import filesstatistics.core.StatisticsReader;
import java.io.IOException;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.List;
import java.util.Map;
import org.jfree.chart.ChartFactory;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.DateAxis;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.StandardXYItemRenderer;
import org.jfree.data.category.DefaultCategoryDataset;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;

/**
 *
 * @author Rafal
 */
public class SimpleChart {

    private StatisticsReader statisticsReader;
    private Map<String, Map<String, Object>> stats;

    public SimpleChart(Map<String, Map<String, Object>> stats) {
        this.stats = stats;
    }

    public JFreeChart createChart() {
        XYSeries series = new XYSeries("XYGraph");
        DateAxis dateAxis = new DateAxis("Date");
        DateFormat chartFormatter = new SimpleDateFormat("yyyy/MM/dd HH:mm");
        dateAxis.setDateFormatOverride(chartFormatter);
        NumberAxis valueAxis = new NumberAxis();


        DefaultCategoryDataset xyDataset = new DefaultCategoryDataset();


        Map<String, Object> infos = stats.get("a.txt");
        for(String file: stats.keySet()){
            Map<String, Object> descriptionMap = stats.get(file);
            
            Integer c = (Integer)descriptionMap.get("M");
            if(c!=null){
                xyDataset.addValue(c, "M", file);
            }
            c = (Integer)descriptionMap.get("C");
            if(c!=null){
                xyDataset.addValue(c, "C", file);

            }c = (Integer) descriptionMap.get("A");
            if(c!=null){
                xyDataset.addValue(c, "A", file);
            }c = (Integer) descriptionMap.get("O");
            if(c!=null){
                xyDataset.addValue(c, "O", file);
            }

        }

        JFreeChart chart = ChartFactory.createStackedBarChart("File system stats",
                "Files",
                "Events",
                xyDataset,
                PlotOrientation.HORIZONTAL,
                true,
                true,
                false);

        return chart;
    }
}

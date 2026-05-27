from traffic_classifier.classifier import classify_rows, classify_rule_based, is_attack_label
from traffic_classifier.features import extract_features
from traffic_classifier.flow import build_flows
from traffic_classifier.ml import apply_model, train_knn
from traffic_classifier.models import PacketRecord


def test_web_flow_rule_classification():
    packets = [
        PacketRecord(1.0, "10.0.0.2", "93.184.216.34", 50000, 443, "TCP", 60, ttl=64, tcp_flags=0x02),
        PacketRecord(1.1, "93.184.216.34", "10.0.0.2", 443, 50000, "TCP", 60, ttl=54, tcp_flags=0x12),
        PacketRecord(1.2, "10.0.0.2", "93.184.216.34", 50000, 443, "TCP", 1200, ttl=64, tcp_flags=0x18),
    ]
    flows = build_flows(packets)

    assert len(flows) == 1
    row = extract_features(flows[0])
    result = classify_rule_based(row)

    assert row["packet_count"] == 3
    assert row["syn_count"] == 2
    assert result.label == "Web"


def test_dns_flow_rule_classification():
    packets = [
        PacketRecord(1.0, "10.0.0.2", "8.8.8.8", 53000, 53, "UDP", 70, ttl=64),
        PacketRecord(1.1, "8.8.8.8", "10.0.0.2", 53, 53000, "UDP", 120, ttl=56),
    ]
    row = extract_features(build_flows(packets)[0])
    result = classify_rule_based(row)

    assert row["avg_packet_size"] == 95
    assert result.label == "DNS"


def test_syn_flood_classification_raises_alert_label():
    packets = [
        PacketRecord(float(idx), "10.0.0.2", "10.0.0.3", 40000, 80, "TCP", 60, ttl=64, tcp_flags=0x02)
        for idx in range(20)
    ]
    row = extract_features(build_flows(packets)[0])
    result = classify_rule_based(row)

    assert result.label == "SYN_Flood"
    assert is_attack_label(result.label)


def test_short_tcp_probe_raises_alert_label():
    row = extract_features(
        build_flows([PacketRecord(1.0, "10.0.0.2", "10.0.0.3", 50000, 8080, "TCP", 60, ttl=64, tcp_flags=0x02)])[0]
    )
    result = classify_rule_based(row)

    assert result.label == "TCP_Probe"
    assert is_attack_label(result.label)


def test_repeated_tcp_probes_to_many_ports_mark_port_scan():
    rows = []
    for idx, port in enumerate([8001, 8002, 8003, 8004, 8005]):
        flow = build_flows(
            [PacketRecord(float(idx), "10.0.0.2", "10.0.0.3", 50000 + idx, port, "TCP", 60, ttl=64, tcp_flags=0x02)]
        )[0]
        rows.append(extract_features(flow))

    classified = classify_rows(rows)

    assert {row["label"] for row in classified} == {"Port_Scan"}
    assert all(is_attack_label(str(row["label"])) for row in classified)


def test_knn_model_predicts_labeled_rows(tmp_path):
    csv_path = tmp_path / "features.csv"
    csv_path.write_text(
        "\n".join(
            [
                "flow_id,src_ip,dst_ip,src_port,dst_port,protocol,packet_count,total_bytes,avg_packet_size,max_packet_size,min_packet_size,duration,avg_interval,interval_variance,avg_ttl,syn_count,ack_count,fin_count,rst_count,psh_count,unique_ports,label,confidence,reason",
                "a,10.0.0.2,8.8.8.8,53000,53,UDP,2,200,100,120,80,0.1,0.1,0,64,0,0,0,0,0,2,DNS,0.92,port 53 DNS traffic",
                "b,10.0.0.2,1.1.1.1,50000,443,TCP,5,5000,1000,1200,60,1.0,0.2,0.1,64,1,4,0,0,2,2,Web,0.88,HTTP/HTTPS port",
            ]
        ),
        encoding="utf-8",
    )
    model = train_knn([csv_path], k=1)
    rows = [
        {
            "src_port": 53000,
            "dst_port": 53,
            "protocol": "UDP",
            "packet_count": 2,
            "total_bytes": 200,
            "avg_packet_size": 100,
            "max_packet_size": 120,
            "min_packet_size": 80,
            "duration": 0.1,
            "avg_interval": 0.1,
            "interval_variance": 0,
            "avg_ttl": 64,
            "syn_count": 0,
            "ack_count": 0,
            "fin_count": 0,
            "rst_count": 0,
            "psh_count": 0,
            "unique_ports": 2,
        }
    ]

    predicted = apply_model(model, rows)

    assert predicted[0]["ml_label"] == "DNS"
    assert predicted[0]["ml_confidence"] == 1.0
